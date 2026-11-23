#include "MainWindow.h"
#include "ScreenCaptureController.h"

#include <QComboBox>
#include <QAction>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QSlider>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>

namespace {
QFrame *panel(const QString &title, QWidget *content, QWidget *parent)
{
    auto *frame = new QFrame(parent);
    frame->setObjectName("panel");
    auto *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    auto *heading = new QLabel(title, frame);
    heading->setObjectName("panelHeading");
    heading->setMinimumHeight(36);
    layout->addWidget(heading);
    layout->addWidget(content, 1);
    return frame;
}

QPushButton *toolButton(const QString &text, QWidget *parent)
{
    auto *button = new QPushButton(text, parent);
    button->setObjectName("toolButton");
    return button;
}
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    captureController = new ScreenCaptureController(this);
    setWindowTitle("MiniOBS");
    resize(1280, 800);
    setMinimumSize(900, 600);

    auto *fileMenu = menuBar()->addMenu("文件");
    fileMenu->addAction("新建场景集合");
    fileMenu->addAction("保存");
    fileMenu->addSeparator();
    fileMenu->addAction("退出", this, &QWidget::close);
    auto *editMenu = menuBar()->addMenu("编辑");
    editMenu->addAction("撤销");
    editMenu->addAction("重做");
    auto *viewMenu = menuBar()->addMenu("视图");
    viewMenu->addAction("重置界面");
    menuBar()->addMenu("配置文件");
    menuBar()->addMenu("场景集合");
    menuBar()->addMenu("工具");
    menuBar()->addMenu("帮助");

    auto *sourceMenu = new QMenu(this);
    screenCaptureAction = sourceMenu->addAction("屏幕捕获");
    screenCaptureAction->setCheckable(true);
    connect(screenCaptureAction, &QAction::triggered, this, [this](bool checked) {
        if (checked)
            startScreenCapture();
        else
            stopScreenCapture();
    });

    auto *central = new QWidget(this);
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(5, 5, 5, 4);
    root->setSpacing(5);

    auto *topBar = new QHBoxLayout;
    auto *addSourceButton = toolButton("＋ 添加来源", central);
    auto *addSourceMenu = new QMenu(addSourceButton);
    addSourceMenu->addAction(screenCaptureAction);
    addSourceButton->setMenu(addSourceMenu);
    topBar->addWidget(addSourceButton);
    topBar->addWidget(toolButton("⚙", central));
    topBar->addStretch();
    auto *live = new QLabel("●  未直播", central);
    live->setObjectName("liveBadge");
    topBar->addWidget(live);
    root->addLayout(topBar);

    auto *mainSplit = new QSplitter(Qt::Vertical, central);
    mainSplit->setChildrenCollapsible(false);
    auto *previewWorkspace = new QWidget(mainSplit);
    auto *previewWorkspaceLayout = new QVBoxLayout(previewWorkspace);
    previewWorkspaceLayout->setContentsMargins(0, 0, 0, 0);
    previewWorkspaceLayout->setSpacing(0);

    auto *scenes = new QListWidget;
    scenes->setObjectName("listPanel");
    scenes->addItems({"主场景", "Starting Soon", "游戏", "聊天", "结束直播"});
    scenes->setCurrentRow(0);
    auto *previewArea = new QWidget;
    previewArea->setObjectName("previewArea");
    auto *previewLayout = new QVBoxLayout(previewArea);
    previewLayout->setContentsMargins(18, 12, 18, 12);
    auto *previewHeader = new QHBoxLayout;
    auto *previewTitle = new QLabel("预览", previewArea);
    previewTitle->setObjectName("mutedLabel");
    previewHeader->addWidget(previewTitle);
    previewHeader->addStretch();
    previewHeader->addWidget(new QLabel("⛶", previewArea));
    previewLayout->addLayout(previewHeader);
    auto *canvas = new QWidget(previewArea);
    canvas->setObjectName("canvas");
    canvas->setMinimumSize(320, 220);
    auto *canvasLayout = new QVBoxLayout(canvas);
    previewLabel = new QLabel("预览画布\n\n从「添加来源」选择「屏幕捕获」", canvas);
    previewLabel->setObjectName("canvasPlaceholder");
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setMinimumSize(320, 220);
    previewLabel->setScaledContents(false);
    canvasLayout->addWidget(previewLabel);
    previewLayout->addWidget(canvas, 1);
    auto *canvasFooter = new QHBoxLayout;
    canvasFooter->addWidget(new QLabel("1920 × 1080", previewArea));
    canvasFooter->addStretch();
    canvasFooter->addWidget(new QLabel("缩放  100%", previewArea));
    previewLayout->addLayout(canvasFooter);
    previewWorkspaceLayout->addWidget(previewArea, 1);

    auto *selectionBar = new QWidget(mainSplit);
    selectionBar->setObjectName("selectionBar");
    auto *selectionLayout = new QHBoxLayout(selectionBar);
    selectionLayout->setContentsMargins(12, 6, 12, 6);
    selectionLayout->addWidget(new QLabel("未选择来源", selectionBar), 1);
    selectionLayout->addWidget(toolButton("⚙ 属性", selectionBar));
    selectionLayout->addWidget(toolButton("▤ 滤镜", selectionBar));
    mainSplit->addWidget(previewWorkspace);
    mainSplit->addWidget(selectionBar);
    mainSplit->setStretchFactor(0, 7);
    mainSplit->setStretchFactor(1, 1);

    auto *sources = new QListWidget;
    sources->setObjectName("listPanel");
    sources->addItems({"显示器采集", "麦克风 / 辅助音频", "桌面音频"});
    auto *sourceArea = new QWidget;
    auto *sourceLayout = new QVBoxLayout(sourceArea);
    sourceLayout->setContentsMargins(5, 5, 5, 5);
    sourceLayout->addWidget(sources);
    auto *sourceActions = new QHBoxLayout;
    sourceActions->addWidget(toolButton("＋", sourceArea));
    sourceActions->addWidget(toolButton("▤", sourceArea));
    sourceActions->addWidget(toolButton("↑", sourceArea));
    sourceActions->addWidget(toolButton("↓", sourceArea));
    sourceActions->addStretch();
    sourceLayout->addLayout(sourceActions);

    auto *mixerContent = new QWidget;
    auto *mixerLayout = new QVBoxLayout(mixerContent);
    mixerLayout->setContentsMargins(0, 0, 0, 0);
    auto *channels = new QHBoxLayout;
    for (const auto &name : {"桌面音频", "麦克风 / 辅助音频"}) {
        auto *channel = new QWidget(mixerContent);
        auto *channelLayout = new QHBoxLayout(channel);
        channelLayout->setContentsMargins(12, 2, 12, 2);
        auto *label = new QLabel(name, channel);
        label->setMinimumWidth(140);
        channelLayout->addWidget(label);
        auto *meter = new QSlider(Qt::Horizontal, channel);
        meter->setRange(0, 100);
        meter->setValue(68);
        meter->setEnabled(false);
        meter->setObjectName("audioMeter");
        channelLayout->addWidget(meter, 1);
        auto *volume = new QLabel("−∞ dB", channel);
        volume->setMinimumWidth(55);
        channelLayout->addWidget(volume);
        channelLayout->addWidget(toolButton("🔊", channel));
        channels->addWidget(channel, 1);
    }
    mixerLayout->addLayout(channels);
    auto *dockArea = new QSplitter(Qt::Horizontal, mainSplit);
    dockArea->setChildrenCollapsible(false);
    auto *sceneArea = new QWidget;
    auto *sceneLayout = new QVBoxLayout(sceneArea);
    sceneLayout->setContentsMargins(5, 5, 5, 5);
    sceneLayout->addWidget(scenes);
    auto *sceneActions = new QHBoxLayout;
    sceneActions->addWidget(toolButton("＋", sceneArea));
    sceneActions->addWidget(toolButton("▤", sceneArea));
    sceneActions->addWidget(toolButton("↑", sceneArea));
    sceneActions->addWidget(toolButton("↓", sceneArea));
    sceneActions->addStretch();
    sceneLayout->addLayout(sceneActions);
    dockArea->addWidget(panel("场景", sceneArea, dockArea));
    dockArea->addWidget(panel("来源", sourceArea, dockArea));
    dockArea->addWidget(panel("音频混音器", mixerContent, dockArea));

    auto *transitionContent = new QWidget;
    auto *transitionLayout = new QVBoxLayout(transitionContent);
    transitionLayout->setContentsMargins(6, 6, 6, 6);
    auto *transition = new QComboBox(transitionContent);
    transition->addItems({"淡入淡出", "切换"});
    transitionLayout->addWidget(transition);
    transitionLayout->addWidget(new QLabel("时长  300 毫秒", transitionContent));
    auto *transitionActions = new QHBoxLayout;
    transitionActions->addStretch();
    transitionActions->addWidget(toolButton("＋", transitionContent));
    transitionActions->addWidget(toolButton("▤", transitionContent));
    transitionLayout->addLayout(transitionActions);
    dockArea->addWidget(panel("场景转场", transitionContent, dockArea));

    auto *controlContent = new QWidget;
    auto *controlLayout = new QVBoxLayout(controlContent);
    controlLayout->setContentsMargins(8, 8, 8, 8);
    auto *startStream = toolButton("开始直播", controlContent);
    startStream->setObjectName("primaryButton");
    auto *startRecord = toolButton("开始录制", controlContent);
    startRecord->setObjectName("primaryButton");
    controlLayout->addWidget(startStream);
    controlLayout->addWidget(startRecord);
    controlLayout->addWidget(toolButton("虚拟摄像头", controlContent));
    controlLayout->addWidget(toolButton("工作室模式", controlContent));
    controlLayout->addWidget(toolButton("设置", controlContent));
    controlLayout->addStretch();
    dockArea->addWidget(panel("控制", controlContent, dockArea));
    dockArea->setStretchFactor(0, 2);
    dockArea->setStretchFactor(1, 2);
    dockArea->setStretchFactor(2, 4);
    dockArea->setStretchFactor(3, 2);
    dockArea->setStretchFactor(4, 2);
    mainSplit->addWidget(dockArea);
    mainSplit->setStretchFactor(2, 3);
    root->addWidget(mainSplit, 1);

    setCentralWidget(central);
    statusBar()->showMessage("就绪  ·  配置文件：未命名  ·  场景集合：默认");

    setStyleSheet(R"(
        QMainWindow, QWidget { background: #202124; color: #e4e5e7; font-family: -apple-system, "Helvetica Neue", sans-serif; font-size: 13px; }
        #menuItem { border: 0; padding: 6px 10px; color: #c7c9cc; }
        #menuItem:hover { background: #303236; }
        #liveBadge { color: #a9adb3; padding: 0 10px; }
        #panel { background: #26282b; border: 1px solid #383a3e; border-radius: 5px; }
        #panelHeading { background: #2b2d30; border-bottom: 1px solid #383a3e; padding-left: 12px; font-weight: 600; }
        #listPanel { background: #222427; border: 0; outline: 0; padding: 5px; }
        #listPanel::item { padding: 8px; border-radius: 3px; }
        #listPanel::item:selected { background: #34373c; color: #fff; border-left: 2px solid #7467ed; }
        #previewArea { background: #26282b; }
        #mutedLabel { color: #9da1a8; }
        #canvas { background: #101113; border: 1px solid #414349; }
        #canvasPlaceholder { color: #737780; font-size: 15px; background: transparent; }
        QPushButton, QComboBox { background: #34363a; border: 1px solid #46484d; border-radius: 4px; padding: 6px 10px; }
        QPushButton:hover, QComboBox:hover { background: #414349; }
        #toolButton { min-width: 28px; padding: 5px 8px; }
        #primaryButton { background: #5147bd; border-color: #6258d1; font-weight: 600; padding: 8px 14px; }
        #primaryButton:hover { background: #6258d1; }
        #audioMeter { accent-color: #52c788; }
        QStatusBar { background: #1b1c1f; color: #92969e; border-top: 1px solid #383a3e; }
        QSplitter::handle { background: #1d1f22; width: 5px; }
    )");

    connect(captureController, &ScreenCaptureController::frameReady, this, [this](const QImage &image) {
        previewLabel->setPixmap(QPixmap::fromImage(image).scaled(previewLabel->size(), Qt::KeepAspectRatio,
                                                                  Qt::SmoothTransformation));
    });
    connect(captureController, &ScreenCaptureController::captureError, this, [this](const QString &message) {
        screenCaptureAction->setChecked(false);
        previewLabel->setText(QStringLiteral("屏幕捕获失败\n\n%1").arg(message));
    });
    connect(captureController, &ScreenCaptureController::captureStopped, this, [this]() {
        screenCaptureAction->setChecked(false);
        previewLabel->setPixmap(QPixmap());
        previewLabel->setText("预览画布\n\n从「添加来源」选择「屏幕捕获」");
    });
}

void MainWindow::startScreenCapture()
{
    previewLabel->setText("正在请求屏幕录制权限…");
    captureController->start();
}

void MainWindow::stopScreenCapture()
{
    captureController->stop();
}
