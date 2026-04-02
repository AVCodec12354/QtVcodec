#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QTLogger.h"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QTLogger::setOutput(ui->text_show_log);
    setValidatorForEditText();
    connectToEncoderUI(this);

    ui->progressBar->setValue(0);
    ui->percent_on_progressBar->setText("0 %");
}

void MainWindow::onPlaying(long currentFrame, long totalFrame) {
    if (videoController) {
        int progress = static_cast<int>((currentFrame * 100) / totalFrame);
        ui->progressBar->setValue(progress);
        ui->percent_on_progressBar->setText(QString::number(progress) + " %");
    }
}

void MainWindow::onFinished() {
    ui->progressBar->setValue(0);
    ui->btn_start->setEnabled(true);
    ui->btn_stop->setEnabled(false);
    QTInfo("Encoder", "Playback/Encoding finished.");
}

void MainWindow::resetUI() {
    // TODO: Reset UI
}

void MainWindow::connectToDecoderUI(MainWindow* window) {

}

void MainWindow::setValidatorForEditText() {
    QRegularExpression regex("[0-9]+");
    ui->width->setValidator(new QRegularExpressionValidator(regex, this));
    ui->height->setValidator(new QRegularExpressionValidator(regex, this));
    ui->fps->setValidator(new QRegularExpressionValidator(regex, this));
    ui->bitdepth->setValidator(new QRegularExpressionValidator(regex, this));
    ui->qp->setValidator(new QRegularExpressionValidator(regex, this));
    ui->level->setValidator(new QRegularExpressionValidator(regex, this));
    ui->band_variable->setValidator(new QRegularExpressionValidator(regex, this));
    ui->max_cu->setValidator(new QRegularExpressionValidator(regex, this));
    ui->speed_cu->setValidator(new QRegularExpressionValidator(regex, this));
    ui->width_of_tile->setValidator(new QRegularExpressionValidator(regex, this));
    ui->height_of_tile->setValidator(new QRegularExpressionValidator(regex, this));
    ui->mastering_display->setValidator(new QRegularExpressionValidator(regex, this));
    ui->content_light_level->setValidator(new QRegularExpressionValidator(regex, this));
}

void MainWindow::connectToEncoderUI(MainWindow* window) {
    encoderViewModel = std::make_shared<EncoderViewModel>();

    // connect button
    connect(ui->btn_inputBrowse, &QPushButton::clicked, window, [this](){
        QString filePath = QFileDialog::getOpenFileName(
                this,
                "Select File",
                lastInputDir,
                "Raw Files (*.mp4 *.mkv *.yuv *y4m);;All Files (*)"
        );
        if (!filePath.isEmpty()) {
            lastInputDir = QFileInfo(filePath).absolutePath();
            ui->input_path->setText(filePath);
        }
    });
    connect(ui->btn_outputBrowse, &QPushButton::clicked, window, [this](){
        QString folderPath = QFileDialog::getExistingDirectory(this, "Select Folder", lastOutputDir);
        if (!folderPath.isEmpty()) {
            lastOutputDir = folderPath;
            ui->output_path->setText(folderPath);
        }
    });
    connect(ui->btn_reconstructedBrowse, &QPushButton::clicked, window, [this](){
        QString folderPath = QFileDialog::getExistingDirectory(this, "Select Folder", lastReconstructedDir);
        if (!folderPath.isEmpty()) {
            lastReconstructedDir = folderPath;
            ui->reconstructed_path->setText(folderPath);
        }
    });
    connect(ui->btn_start, &QPushButton::clicked, window, [this](){
        if (!ui->input_path->text().isEmpty()) {
            encoderViewModel->start();
            ui->btn_start->setEnabled(false);
            ui->btn_stop->setEnabled(true);

            videoController = new VideoController(this, ui->openGLWidget);
            videoController->setListener(this);

            videoController->loadVideo(ui->input_path->text());
            videoController->play();
            QTInfo("Encoder", "Start encoding...");
        } else {
            QTError("Encoder", "Input path is empty!");
        }
    });
    connect(ui->btn_save_config, &QPushButton::clicked, window, [this](){
        QTDebug("Encoder", "btn_save_config clicked!");
    });
    connect(ui->btn_stop, &QPushButton::clicked, window, [this](){
        encoderViewModel->stop();
        videoController->stop();
    });
    connect(ui->btn_reset, &QPushButton::clicked, window, [this](){
        QTDebug("Encoder", "btn_reset clicked!");
    });

    // Basic Settings:
    connect(ui->width, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->width->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid width input!");
        } else {
            encoderViewModel->setWidth(value);
        }
    });
    connect(ui->height, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->height->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid height input!");
        } else {
            encoderViewModel->setHeight(value);
        }
    });
    connect(ui->fps, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->fps->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid FPS input!");
        } else {
            encoderViewModel->setFPS(value);
        }
    });
    connect(ui->bitdepth, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                bool isSuccess;
                int value = ui->bitdepth->currentText().toInt(&isSuccess);
                if (!isSuccess) {
                    QTError("Encoder", "Invalid bitdepth input!");
                } else {
                    encoderViewModel->setBitDepth(value);
                }
            });
    connect(ui->colorspace, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                encoderViewModel->setColorSpace(text.toStdString());
            });

    // Bitrate and Quality
    connect(ui->enableBitrateABR, &QCheckBox::toggled,
            this, [this](bool checked){
                encoderViewModel->enableBitrateABR(checked);
            });
    connect(ui->qp, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->qp->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid Quantization Parameter input!");
        } else {
            encoderViewModel->setQuantizationParameters(value);
        }
    });
    connect(ui->profile, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                encoderViewModel->setProfile(text.toStdString());
            });
    connect(ui->level, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                bool isSuccess;
                int value = ui->level->currentText().toInt(&isSuccess);
                if (!isSuccess) {
                    QTError("Encoder", "Invalid level input!");
                } else {
                    encoderViewModel->setLevel(value);
                }
            });
    connect(ui->family, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                encoderViewModel->setFamily(text.toStdString());
            });
    connect(ui->band_variable, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                bool isSuccess;
                int value = ui->band_variable->currentText().toInt(&isSuccess);
                if (!isSuccess) {
                    QTError("Encoder", "Invalid band input!");
                } else {
                    encoderViewModel->setBand(value);
                }
            });

    // Optimize
    connect(ui->max_cu, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->max_cu->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid Max CU input!");
        } else {
            encoderViewModel->setMaxCU(value);
        }
    });
    connect(ui->speed_cu, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->speed_cu->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid Speed CU input!");
        } else {
            encoderViewModel->setSpeedCU(value);
        }
    });
    connect(ui->width_of_tile, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->width_of_tile->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid Width Of Tile input!");
        } else {
            encoderViewModel->setWidthOfTile(value);
        }
    });
    connect(ui->height_of_tile, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->height_of_tile->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid Height Of Tile input!");
        } else {
            encoderViewModel->setHeightOfTile(value);
        }
    });

    // Color Metadata
    connect(ui->primaries, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
                encoderViewModel->setPrimaries(text.toStdString());
            });
    connect(ui->transfer, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
                encoderViewModel->setTransfer(text.toStdString());
            });
    connect(ui->matrix, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
                encoderViewModel->setMatrix(text.toStdString());
            });
    connect(ui->range, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
                encoderViewModel->setRange(text.toStdString());
            });
    connect(ui->mastering_display, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->mastering_display->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid Mastering Display input!");
        } else {
            encoderViewModel->setMasteringDisplay(value);
        }
    });
    connect(ui->content_light_level, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->content_light_level->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid Content Light Level input!");
        } else {
            encoderViewModel->setContentLightLevel(value);
        }
    });
}

MainWindow::~MainWindow()
{
    QTLogger::setOutput(nullptr);
    delete ui;
}
