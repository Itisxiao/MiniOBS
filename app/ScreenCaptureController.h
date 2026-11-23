#pragma once

#include <QObject>
#include <QImage>

#include <memory>

class ScreenCaptureController final : public QObject {
    Q_OBJECT
public:
    explicit ScreenCaptureController(QObject *parent = nullptr);
    ~ScreenCaptureController() override;

    void start();
    void stop();

signals:
    void frameReady(const QImage &image);
    void captureError(const QString &message);
    void captureStopped();

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
