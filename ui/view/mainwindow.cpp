#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connectToEncoderUI(this);
}

void MainWindow::connectToDecoderUI(MainWindow* window) {

}

void MainWindow::connectToEncoderUI(MainWindow* window) {
    encoderViewModel = std::make_shared<EncoderViewModel>();

    // connect button
    connect(ui->btn_inputBrowse, &QPushButton::clicked, window, [this](){
        qDebug() << "btn_inputBrowse Clicked!";
        ui->input_path->setText("/btn_inputBrowse Clicked!");
    });
    connect(ui->btn_outputBrowse, &QPushButton::clicked, window, [this](){
        qDebug() << "btn_outputBrowse Clicked!";
        ui->output_path->setText("/btn_outputBrowse Clicked!");
    });
    connect(ui->btn_reconstructedBrowse, &QPushButton::clicked, window, [this](){
        qDebug() << "btn_reconstructedBrowse Clicked!";
        ui->reconstructed_path->setText("/btn_reconstructedBrowse Clicked!");
    });
    connect(ui->btn_start, &QPushButton::clicked, window, [this](){
        encoderViewModel->start();
    });
    connect(ui->btn_save_config, &QPushButton::clicked, window, [this](){
        qDebug() << "btn_save_config Clicked!";
    });
    connect(ui->btn_stop, &QPushButton::clicked, window, [this](){
        encoderViewModel->stop();
    });
    connect(ui->btn_reset, &QPushButton::clicked, window, [this](){
        qDebug() << "btn_reset Clicked!";
    });

    // Basic Settings:
    connect(ui->width, &QLineEdit::textChanged, window, [this](const QString &text){
        encoderViewModel->setWidth(0);
    });
    connect(ui->height, &QLineEdit::textChanged, window, [this](const QString &text){
        encoderViewModel->setHeight(0);
    });
    connect(ui->fps, &QLineEdit::textChanged, window, [this](const QString &text){
        encoderViewModel->setFPS(0);
    });
    connect(ui->bitdepth, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                encoderViewModel->setBitDepth(0);
            });
    connect(ui->colorspace, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                encoderViewModel->setColorSpace("BT709");
            });

    // Bitrate and Quality
    connect(ui->enableBitrateABR, &QCheckBox::toggled,
            this, [this](bool checked){
                encoderViewModel->enableBitrateABR(false);
            });
    connect(ui->qp, &QLineEdit::textChanged, window, [this](const QString &text){
        encoderViewModel->setQuantizationParameters(0);
    });
    connect(ui->profile, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                encoderViewModel->setProfile("AAA");
            });
    connect(ui->level, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                encoderViewModel->setLevel(0);
            });
    connect(ui->family, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                encoderViewModel->setFamily("AAAA");
            });
    connect(ui->band_variable, &QComboBox::currentTextChanged,
            this, [this](const QString &text){
                encoderViewModel->setBand(0);
            });

    // Optimize
    connect(ui->max_cu, &QLineEdit::textChanged, window, [this](const QString &text){
        encoderViewModel->setMaxCU(25);
    });
    connect(ui->speed_cu, &QLineEdit::textChanged, window, [this](const QString &text){
        encoderViewModel->setSpeedCU(25);
    });
    connect(ui->width_of_tile, &QLineEdit::textChanged, window, [this](const QString &text){
        encoderViewModel->setWidthOfTile(1);
    });
    connect(ui->height_of_tile, &QLineEdit::textChanged, window, [this](const QString &text){
        encoderViewModel->setHeightOfTile(1);
    });

    // Color Metadata
    connect(ui->primaries, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
                encoderViewModel->setPrimaries("AAAA");
            });
    connect(ui->transfer, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
                encoderViewModel->setTransfer("AAAA");
            });
    connect(ui->matrix, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
                encoderViewModel->setMatrix("AAAA");
            });
    connect(ui->range, &QComboBox::currentTextChanged,
            this, [this](const QString &text) {
                encoderViewModel->setRange("AAAA");
            });
    connect(ui->mastering_display, &QLineEdit::textChanged, window, [this](const QString &text){
        encoderViewModel->setMasteringDisplay(0);
    });
    connect(ui->content_light_level, &QLineEdit::textChanged, window, [this](const QString &text){
        encoderViewModel->setContentLightLevel(0);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
