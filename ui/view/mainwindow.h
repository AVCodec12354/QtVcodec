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
    std::shared_ptr<Ui::MainWindow> ui;
    QString lastInputDir, lastOutputDir, lastReconstructedDir;

    std::shared_ptr<VideoRenderer> videoRenderer;
    std::shared_ptr<EncoderViewModel> encoderViewModel;

    void resetEncoderUI();
    void setValidatorForEditText();
    void connectToDecoderUI(MainWindow* window);
    void connectToEncoderUI(MainWindow* window);
};

#endif // MAINWINDOW_H