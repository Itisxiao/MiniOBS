#pragma once

#include <QMainWindow>
#include <QImage>

class QLabel;
class QAction;
class ScreenCaptureController;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void startScreenCapture();
    void stopScreenCapture();

    QLabel *previewLabel = nullptr;
    QAction *screenCaptureAction = nullptr;
    ScreenCaptureController *captureController = nullptr;
};
