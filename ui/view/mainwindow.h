#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QRegularExpressionValidator>

#include <iostream>
#include "EncoderTabViewModel.h"
#include "VideoRenderer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void parseFileNameAndSetUI(const QString &filePath);

    std::unique_ptr<Ui::MainWindow> ui;
    QString lastInputDir, lastOutputDir, lastReconstructedDir;

    std::unique_ptr<EncoderTabViewModel> encoderTabViewModel;
    void resetEncoderUI();
    void setValidatorForEditText();
    void connectToDecoderUI(MainWindow* window);
    void connectToEncoderUI(MainWindow* window);
};

#endif // MAINWINDOW_H