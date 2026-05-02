#include <EncoderTabViewModel.h>
#include <QTLogger.h>

#include <YUVSource.h>
#include <Y4MSource.h>
#include <chrono>

EncoderTabViewModel::EncoderTabViewModel(VideoGLWidget *glWidget) {
    videoWidget = qobject_cast<VideoGLWidget*>(glWidget);
    qRegisterMetaType<std::shared_ptr<Qv2Buffer>>("std::shared_ptr<Qv2Buffer>");
}

EncoderTabViewModel::~EncoderTabViewModel() {
    mIsRunning = false;
    if (mRenderThread.joinable()) {
        mRenderThread.join();
    }
}

void EncoderTabViewModel::start(const std::string &file) {
    if (mIsRunning) return;
    if (std::filesystem::path(file).extension() == ".y4m") {
        rawSource = std::make_shared<Y4MSource>();
    } else {
        rawSource = std::make_shared<YUVSource>();
    }

    rawSource->setDataSource(file, mWidth, mHeight, mBitDepth, QV2_CF_YCBCR422_10LE);
    mIsRunning = true;
    int fps = (mFPS > 0) ? mFPS : 25;
    auto frameDuration = std::chrono::milliseconds(1000 / fps);

    mRenderThread = std::thread([this, frameDuration]() {
        while (mIsRunning) {
            auto nextFrameTime = std::chrono::steady_clock::now() + frameDuration;
            auto frame = rawSource->getBuffer();
            if (frame) {
                QMetaObject::invokeMethod(videoWidget, "bindBuffer",
                                          Qt::QueuedConnection,
                                          Q_ARG(std::shared_ptr<Qv2Buffer>, frame));
                // mEncoder->encodeFrame(frame);
            } else {
                mIsRunning = false;
                break;
            }
            std::this_thread::sleep_until(nextFrameTime);
        }
    });
}

void EncoderTabViewModel::stop() {
    mIsRunning = false;
    rawSource.reset();
    if (mRenderThread.joinable()) {
        mRenderThread.join();
    }
}

void EncoderTabViewModel::setWidth(int value) {
    mWidth = value;
}

void EncoderTabViewModel::setHeight(int value) {
    mHeight = value;
}

void EncoderTabViewModel::setFPS(int value) {
    mFPS = value;
}

void EncoderTabViewModel::setBitDepth(int value) {
    mBitDepth = value;
}

void EncoderTabViewModel::setPixelFormat(string value) {
    // TODO
}

void EncoderTabViewModel::enableBitrateABR(bool value) {
    // TODO
}

void EncoderTabViewModel::setQuantizationParameters(int value) {
    // TODO
}

void EncoderTabViewModel::setProfile(string value) {
    // TODO
}

void EncoderTabViewModel::setLevel(int value) {
    // TODO
}

void EncoderTabViewModel::setFamily(string value) {
    // TODO
}

void EncoderTabViewModel::setBand(int value) {
    // TODO
}

void EncoderTabViewModel::setMaxCU(int value) {
    // TODO
}

void EncoderTabViewModel::setSpeedCU(int value) {
    // TODO
}

void EncoderTabViewModel::setWidthOfTile(int value) {
    // TODO
}

void EncoderTabViewModel::setHeightOfTile(int value) {
    // TODO
}

void EncoderTabViewModel::setPrimaries(string value) {
    // TODO
}

void EncoderTabViewModel::setTransfer(string value) {
    // TODO
}

void EncoderTabViewModel::setMatrix(string value) {
    // TODO
}

void EncoderTabViewModel::setRange(string value) {
    // TODO
}

void EncoderTabViewModel::setMasteringDisplay(int value) {
    // TODO
}

void EncoderTabViewModel::setContentLightLevel(int value) {
    // TODO
}