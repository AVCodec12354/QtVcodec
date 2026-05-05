#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QTLogger.h"

#define LOG_TAG "MAIN_WINDOW"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(std::make_unique<Ui::MainWindow>())
{
    ui->setupUi(this);
    ui->openGLWidget->setMinimumSize(1, 1);
    ui->openGLWidget->show();
    QTLogger::setOutput(ui->text_show_log);
    setValidatorForEditText();
    connectToEncoderUI(this);
    mEncoderTabViewModel = std::make_unique<EncoderTabViewModel>(ui->openGLWidget);
    connect(mEncoderTabViewModel.get(), &EncoderTabViewModel::playing,
            this, &MainWindow::onPlaying);
    connect(mEncoderTabViewModel.get(), &EncoderTabViewModel::finished,
            this, &MainWindow::onFinished);

    ui->matrix->setCurrentIndex(1);
    ui->range->setCurrentIndex(1);
    QString primaries = ui->primaries->currentText();
    mEncoderTabViewModel->setPrimaries(primaries.toStdString());
    QString transfer = ui->transfer->currentText();
    mEncoderTabViewModel->setTransfer(transfer.toStdString());
    QString matrix = ui->matrix->currentText();
    mEncoderTabViewModel->setMatrix(matrix.toStdString());
    QString range = ui->range->currentText();
    mEncoderTabViewModel->setRange(range.toStdString());
}

void MainWindow::onPlaying(long currentFrame, long totalFrame) {
    int progress = static_cast<int>((currentFrame * 100) / totalFrame);
    ui->progressBar->setValue(progress);
    ui->percent_on_progressBar->setText(QString::number(progress) + "%");
    ui->encoded_frame->setText("Frames: " + QString::number(currentFrame) + "/" + QString::number(totalFrame));
}

void MainWindow::onFinished() {
    ui->btn_start->setEnabled(true);
    QTInfo("Encoder", "Playback/Encoding finished.");
}

void MainWindow::parseFileNameAndSetUI(const QString &filePath) {
    QString width = "1920", height = "1080", bit = "8", fps = "30";
    QStringList parts = QFileInfo(filePath).baseName().toLower().split('_');
    if (parts.size() < 5) return;

    QStringList res = parts[1].split('x');
    if (res.size() == 2) {
        width = res[0]; height = res[1];
    }
    bit = parts[3].remove("bit").remove("b");
    fps = parts[4].remove("fps");

    ui->width->setText(width);
    ui->height->setText(height);
    ui->fps->setText(fps);
    ui->bitdepth->setCurrentText(bit);

    QTInfo("UI", QString("Config Set: %1x%2, %3-bit, %4 FPS")
        .arg(width, height, bit, fps));
}

void MainWindow::resetEncoderUI() {
    ui->input_path->setText("");
    ui->output_path->setText("");
    ui->reconstructed_path->setText("");
    ui->encoded_frame->setText("Frames: 0/0");
    ui->time_encoding->setText("Time Encoding: 00:00:00");
    ui->progressBar->setValue(0);
    ui->percent_on_progressBar->setText("0%");
    // Settings
    ui->width->setText("");
    ui->height->setText("");
    ui->fps->setText("");
    ui->bitdepth->setCurrentIndex(0);
    ui->colorspace->setCurrentIndex(0);
    ui->enableBitrateABR->setChecked(false);
    ui->qp->setText("");
    ui->profile->setCurrentIndex(0);
    ui->level->setCurrentIndex(0);
    ui->family->setCurrentIndex(0);
    ui->band_variable->setCurrentIndex(0);
    ui->max_cu->setText("");
    ui->speed_cu->setText("");
    ui->width_of_tile->setText("");
    ui->height_of_tile->setText("");
    ui->primaries->setCurrentIndex(0);
    ui->transfer->setCurrentIndex(0);
    ui->matrix->setCurrentIndex(1);
    ui->range->setCurrentIndex(1);
    ui->mastering_display->setText("");
    ui->content_light_level->setText("");
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
            parseFileNameAndSetUI(filePath);
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
    connect(ui->delete_button, &QPushButton::clicked, window, [this](){
        ui->text_show_log->setText("");
    });
    connect(ui->save_log_button, &QPushButton::clicked, window, [this](){
        QTDebug("Encoder", "save_log_button clicked");
    });

    connect(ui->btn_start, &QPushButton::clicked, window, [this](){
        if (!ui->input_path->text().isEmpty()) {
            std::string inputPath = ui->input_path->text().toStdString();
            mEncoderTabViewModel->start(inputPath);
            ui->btn_start->setEnabled(false);
        } else {
            QTError("Encoder", "Input path is empty!");
        }
    });
    connect(ui->btn_save_config, &QPushButton::clicked, window, [this](){
        QTDebug("Encoder", "btn_save_config clicked!");
    });
    connect(ui->btn_stop, &QPushButton::clicked, window, [this](){
        mEncoderTabViewModel->stop();
        ui->btn_start->setEnabled(true);
    });
    connect(ui->btn_reset, &QPushButton::clicked, window, [this](){
        resetEncoderUI();
        QTDebug("Encoder", "btn_reset clicked!");
    });

    // Basic Settings:
    connect(ui->width, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->width->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid width input!");
        } else {
            mEncoderTabViewModel->setWidth(value);
        }
    });
    connect(ui->height, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->height->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid height input!");
        } else {
            mEncoderTabViewModel->setHeight(value);
        }
    });
    connect(ui->fps, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->fps->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid FPS input!");
        } else {
            mEncoderTabViewModel->setFPS(value);
        }
    });
    connect(ui->bitdepth, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                bool isSuccess;
                int value = ui->bitdepth->currentText().toInt(&isSuccess);
                if (!isSuccess) {
                    QTError("Encoder", "Invalid bitdepth input!");
                } else {
                    mEncoderTabViewModel->setBitDepth(value);
                }
            });
    connect(ui->colorspace, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                mEncoderTabViewModel->setPixelFormat(text.toStdString());
            });

    // Bitrate and Quality
    connect(ui->enableBitrateABR, &QCheckBox::toggled,
            this, [this](bool checked){
                mEncoderTabViewModel->enableBitrateABR(checked);
            });
    connect(ui->qp, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->qp->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid Quantization Parameter input!");
        } else {
            mEncoderTabViewModel->setQuantizationParameters(value);
        }
    });
    connect(ui->profile, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                mEncoderTabViewModel->setProfile(text.toStdString());
            });
    connect(ui->level, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                bool isSuccess;
                int value = ui->level->currentText().toInt(&isSuccess);
                if (!isSuccess) {
                    QTError("Encoder", "Invalid level input!");
                } else {
                    mEncoderTabViewModel->setLevel(value);
                }
            });
    connect(ui->family, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                mEncoderTabViewModel->setFamily(text.toStdString());
            });
    connect(ui->band_variable, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                bool isSuccess;
                int value = ui->band_variable->currentText().toInt(&isSuccess);
                if (!isSuccess) {
                    QTError("Encoder", "Invalid band input!");
                } else {
                    mEncoderTabViewModel->setBand(value);
                }
            });

    // Optimize
    connect(ui->max_cu, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->max_cu->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid Max CU input!");
        } else {
            mEncoderTabViewModel->setMaxCU(value);
        }
    });
    connect(ui->speed_cu, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->speed_cu->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid Speed CU input!");
        } else {
            mEncoderTabViewModel->setSpeedCU(value);
        }
    });
    connect(ui->width_of_tile, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->width_of_tile->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid Width Of Tile input!");
        } else {
            mEncoderTabViewModel->setWidthOfTile(value);
        }
    });
    connect(ui->height_of_tile, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->height_of_tile->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid Height Of Tile input!");
        } else {
            mEncoderTabViewModel->setHeightOfTile(value);
        }
    });

    // Color Metadata
    connect(ui->primaries, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
                mEncoderTabViewModel->setPrimaries(text.toStdString());
            });
    connect(ui->transfer, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
                mEncoderTabViewModel->setTransfer(text.toStdString());
            });
    connect(ui->matrix, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
                mEncoderTabViewModel->setMatrix(text.toStdString());
            });
    connect(ui->range, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
                mEncoderTabViewModel->setRange(text.toStdString());
            });
    connect(ui->mastering_display, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->mastering_display->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid Mastering Display input!");
        } else {
            mEncoderTabViewModel->setMasteringDisplay(value);
        }
    });
    connect(ui->content_light_level, &QLineEdit::textChanged, window, [this](const QString &text){
        bool isSuccess;
        int value = ui->content_light_level->text().toInt(&isSuccess);
        if (!isSuccess) {
            QTError("Encoder", "Invalid Content Light Level input!");
        } else {
            mEncoderTabViewModel->setContentLightLevel(value);
        }
    });
}

MainWindow::~MainWindow() {
    QTLogger::setOutput(nullptr);
}
