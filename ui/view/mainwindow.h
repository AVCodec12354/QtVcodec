#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QRegularExpressionValidator>

#include "EncoderViewModel.h"
#include "VideoController.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void connectToDecoderUI(MainWindow* window);
    void connectToEncoderUI(MainWindow* window);
    void setValidatorForEditText();

private:
    Ui::MainWindow *ui;
    VideoController *videoController;
    QString lastInputDir, lastOutputDir, lastReconstructedDir;

    std::shared_ptr<EncoderViewModel> encoderViewModel;
};

#endif // MAINWINDOW_H