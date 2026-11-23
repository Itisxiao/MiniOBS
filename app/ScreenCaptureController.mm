#import "ScreenCaptureController.h"

#import <ScreenCaptureKit/ScreenCaptureKit.h>
#import <CoreImage/CoreImage.h>
#import <CoreVideo/CoreVideo.h>

#include <QMetaObject>
#include <QPointer>

@interface MiniOBSStreamOutput : NSObject <SCStreamOutput>
@property(nonatomic, assign) ScreenCaptureController *owner;
@property(nonatomic, strong) CIContext *context;
@end

@implementation MiniOBSStreamOutput
- (instancetype)initWithOwner:(ScreenCaptureController *)owner
{
    self = [super init];
    if (self) {
        _owner = owner;
        _context = [CIContext contextWithOptions:nil];
    }
    return self;
}

- (void)stream:(SCStream *)stream didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer ofType:(SCStreamOutputType)type
{
    Q_UNUSED(stream);
    if (type != SCStreamOutputTypeScreen || !CMSampleBufferIsValid(sampleBuffer))
        return;
    CVPixelBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    if (!pixelBuffer)
        return;
    CIImage *image = [CIImage imageWithCVPixelBuffer:pixelBuffer];
    CGImageRef cgImage = [self.context createCGImage:image fromRect:image.extent];
    if (!cgImage)
        return;
    QImage frame(static_cast<int>(CGImageGetWidth(cgImage)), static_cast<int>(CGImageGetHeight(cgImage)),
                 QImage::Format_RGBA8888);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef bitmap = CGBitmapContextCreate(frame.bits(), frame.width(), frame.height(), 8,
                                                frame.bytesPerLine(), colorSpace,
                                                kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    if (bitmap) {
        CGContextDrawImage(bitmap, CGRectMake(0, 0, frame.width(), frame.height()), cgImage);
        CGContextRelease(bitmap);
        QPointer<ScreenCaptureController> guard(self.owner);
        if (guard) {
            QMetaObject::invokeMethod(guard, [guard, frame]() {
                if (guard)
                    emit guard->frameReady(frame);
            }, Qt::QueuedConnection);
        }
    }
    CGColorSpaceRelease(colorSpace);
    CGImageRelease(cgImage);
}
@end

struct ScreenCaptureController::Impl {
    SCStream *stream = nil;
    MiniOBSStreamOutput *output = nil;
    dispatch_queue_t queue = dispatch_queue_create("com.miniobs.screen-capture", DISPATCH_QUEUE_SERIAL);
    bool stopping = false;
};

ScreenCaptureController::ScreenCaptureController(QObject *parent)
    : QObject(parent), impl(std::make_unique<Impl>())
{
}

ScreenCaptureController::~ScreenCaptureController()
{
    stop();
}

void ScreenCaptureController::start()
{
    if (impl->stream)
        return;
    impl->stopping = false;
    QPointer<ScreenCaptureController> guard(this);
    [SCShareableContent getShareableContentWithCompletionHandler:^(SCShareableContent *content, NSError *error) {
        if (!guard)
            return;
        if (error || content.displays.count == 0) {
            const QString message = error ? QString::fromNSString(error.localizedDescription)
                                          : QStringLiteral("没有可采集的显示器");
            QMetaObject::invokeMethod(guard, [guard, message]() {
                if (guard) emit guard->captureError(message);
            }, Qt::QueuedConnection);
            return;
        }

        SCDisplay *display = content.displays.firstObject;
        SCContentFilter *filter = [[SCContentFilter alloc] initWithDisplay:display excludingWindows:@[]];
        SCStreamConfiguration *configuration = [[SCStreamConfiguration alloc] init];
        configuration.width = display.width;
        configuration.height = display.height;
        configuration.minimumFrameInterval = CMTimeMake(1, 30);
        configuration.queueDepth = 5;
        configuration.pixelFormat = kCVPixelFormatType_32BGRA;

        guard->impl->output = [[MiniOBSStreamOutput alloc] initWithOwner:guard.data()];
        guard->impl->stream = [[SCStream alloc] initWithFilter:filter configuration:configuration delegate:nil];
        NSError *outputError = nil;
        if (![guard->impl->stream addStreamOutput:guard->impl->output type:SCStreamOutputTypeScreen
                             sampleHandlerQueue:guard->impl->queue error:&outputError]) {
            const QString message = QString::fromNSString(outputError.localizedDescription);
            QMetaObject::invokeMethod(guard, [guard, message]() {
                if (guard) emit guard->captureError(message);
            }, Qt::QueuedConnection);
            return;
        }
        [guard->impl->stream startCaptureWithCompletionHandler:^(NSError *startError) {
            if (!guard)
                return;
            if (startError) {
                const QString message = QString::fromNSString(startError.localizedDescription);
                QMetaObject::invokeMethod(guard, [guard, message]() {
                    if (guard) emit guard->captureError(message);
                }, Qt::QueuedConnection);
            }
        }];
    }];
}

void ScreenCaptureController::stop()
{
    if (!impl->stream || impl->stopping)
        return;
    impl->stopping = true;
    QPointer<ScreenCaptureController> guard(this);
    SCStream *stream = impl->stream;
    [stream stopCaptureWithCompletionHandler:^(NSError *error) {
        Q_UNUSED(error);
        if (!guard)
            return;
        QMetaObject::invokeMethod(guard, [guard]() {
            if (!guard)
                return;
            guard->impl->stream = nil;
            guard->impl->output = nil;
            guard->impl->stopping = false;
            emit guard->captureStopped();
        }, Qt::QueuedConnection);
    }];
}
