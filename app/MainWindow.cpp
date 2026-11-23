#include "MainWindow.h"

#include <QComboBox>
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

    auto *central = new QWidget(this);
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(10, 8, 10, 8);
    root->setSpacing(8);

    auto *topBar = new QHBoxLayout;
    topBar->addSpacing(18);
    topBar->addWidget(toolButton("＋ 添加来源", central));
    topBar->addWidget(toolButton("⚙ 设置", central));
    topBar->addStretch();
    auto *live = new QLabel("●  未直播", central);
    live->setObjectName("liveBadge");
    topBar->addWidget(live);
    root->addLayout(topBar);

    auto *body = new QSplitter(Qt::Horizontal, central);
    body->setChildrenCollapsible(false);
    auto *leftWorkspace = new QWidget(body);
    auto *leftLayout = new QVBoxLayout(leftWorkspace);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(8);

    auto *scenes = new QListWidget;
    scenes->setObjectName("listPanel");
    scenes->addItems({"主场景", "Starting Soon", "游戏", "聊天", "结束直播"});
    scenes->setCurrentRow(0);
    auto *sceneArea = new QWidget;
    auto *sceneLayout = new QVBoxLayout(sceneArea);
    sceneLayout->setContentsMargins(8, 8, 8, 8);
    sceneLayout->addWidget(scenes);
    auto *sceneActions = new QHBoxLayout;
    sceneActions->addWidget(toolButton("＋", sceneArea));
    sceneActions->addWidget(toolButton("－", sceneArea));
    sceneActions->addWidget(toolButton("⚙", sceneArea));
    sceneLayout->addLayout(sceneActions);
    auto *bottomWorkspace = new QSplitter(Qt::Horizontal, leftWorkspace);
    bottomWorkspace->setChildrenCollapsible(false);
    bottomWorkspace->addWidget(panel("场景", sceneArea, bottomWorkspace));

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
    auto *placeholder = new QLabel("预览画布\n\n添加来源以开始构建场景", canvas);
    placeholder->setObjectName("canvasPlaceholder");
    placeholder->setAlignment(Qt::AlignCenter);
    canvasLayout->addWidget(placeholder);
    previewLayout->addWidget(canvas, 1, Qt::AlignCenter);
    auto *canvasFooter = new QHBoxLayout;
    canvasFooter->addWidget(new QLabel("1920 × 1080", previewArea));
    canvasFooter->addStretch();
    canvasFooter->addWidget(new QLabel("缩放  100%", previewArea));
    previewLayout->addLayout(canvasFooter);
    auto *previewPanel = panel("视频预览", previewArea, leftWorkspace);
    leftLayout->addWidget(previewPanel, 7);

    auto *sources = new QListWidget;
    sources->setObjectName("listPanel");
    sources->addItems({"显示器采集", "麦克风 / 辅助音频", "桌面音频"});
    auto *sourceArea = new QWidget;
    auto *sourceLayout = new QVBoxLayout(sourceArea);
    sourceLayout->setContentsMargins(8, 8, 8, 8);
    sourceLayout->addWidget(sources);
    auto *sourceActions = new QHBoxLayout;
    sourceActions->addWidget(toolButton("＋", sourceArea));
    sourceActions->addWidget(toolButton("－", sourceArea));
    sourceActions->addWidget(toolButton("↑", sourceArea));
    sourceActions->addWidget(toolButton("↓", sourceArea));
    sourceLayout->addLayout(sourceActions);
    bottomWorkspace->addWidget(panel("来源", sourceArea, bottomWorkspace));

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
    bottomWorkspace->addWidget(panel("音频混音器", mixerContent, bottomWorkspace));
    bottomWorkspace->setStretchFactor(0, 2);
    bottomWorkspace->setStretchFactor(1, 3);
    bottomWorkspace->setStretchFactor(2, 5);
    leftLayout->addWidget(bottomWorkspace, 3);
    body->addWidget(leftWorkspace);

    auto *controlContent = new QWidget;
    auto *controlLayout = new QVBoxLayout(controlContent);
    controlLayout->setContentsMargins(10, 10, 10, 10);
    controlLayout->setSpacing(10);
    controlLayout->addWidget(new QLabel("直播", controlContent));
    controlLayout->addWidget(toolButton("设置直播信息", controlContent));
    controlLayout->addWidget(new QLabel("录制", controlContent));
    auto *recordPath = new QLabel("保存位置\n~/Movies", controlContent);
    recordPath->setObjectName("mutedLabel");
    controlLayout->addWidget(recordPath);
    controlLayout->addWidget(new QLabel("场景转场", controlContent));
    auto *transition = new QComboBox(controlContent);
    transition->addItems({"淡入淡出", "切换"});
    controlLayout->addWidget(transition);
    controlLayout->addWidget(toolButton("工作室模式", controlContent));
    controlLayout->addStretch();
    body->addWidget(panel("控制面板", controlContent, body));
    body->setStretchFactor(0, 5);
    body->setStretchFactor(1, 1);
    root->addWidget(body, 1);

    auto *controls = new QHBoxLayout;
    auto *startStream = toolButton("开始直播", central);
    startStream->setObjectName("primaryButton");
    auto *startRecord = toolButton("开始录制", central);
    startRecord->setObjectName("primaryButton");
    controls->addWidget(startStream);
    controls->addWidget(startRecord);
    controls->addStretch();
    root->addLayout(controls);

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
        #canvasPlaceholder { color: #737780; font-size: 15px; }
        QPushButton, QComboBox { background: #34363a; border: 1px solid #46484d; border-radius: 4px; padding: 6px 10px; }
        QPushButton:hover, QComboBox:hover { background: #414349; }
        #toolButton { min-width: 28px; padding: 5px 8px; }
        #primaryButton { background: #5147bd; border-color: #6258d1; font-weight: 600; padding: 8px 14px; }
        #primaryButton:hover { background: #6258d1; }
        #audioMeter { accent-color: #52c788; }
        QStatusBar { background: #1b1c1f; color: #92969e; border-top: 1px solid #383a3e; }
        QSplitter::handle { background: #1d1f22; width: 5px; }
    )");
}
