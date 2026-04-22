#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QRegularExpressionValidator>

#include <iostream>
#include "EncoderViewModel.h"
#include "VideoRenderer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public VideoRenderer::Listener {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void onPlaying(long currentFrame, long totalFrame) override;
    void onFinished() override;

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QString lastInputDir, lastOutputDir, lastReconstructedDir;

    std::unique_ptr<VideoRenderer> videoRenderer;
    std::unique_ptr<EncoderViewModel> encoderViewModel;
    std::shared_ptr<Qv2Component> mEncoder;

    void resetEncoderUI();
    void setValidatorForEditText();
    void connectToDecoderUI(MainWindow* window);
    void connectToEncoderUI(MainWindow* window);
};

#endif // MAINWINDOW_H