#include <EncoderTabViewModel.h>
#include <QTLogger.h>

#include <YUVSource.h>
#include <Y4MSource.h>
#include <chrono>

EncoderTabViewModel::~EncoderTabViewModel() {
    mIsRunning = false;
    if (mRenderThread.joinable()) {
        mRenderThread.join();
    }
    if (mVideoWidget) {
        delete mVideoWidget;
    }
}

void EncoderTabViewModel::start(const std::string &file) {
    if (mIsRunning) return;
    if (mRenderThread.joinable()) { mRenderThread.join(); }

    if (std::filesystem::path(file).extension() == ".y4m") {
        mRawSource = std::make_shared<Y4MSource>();
    } else {
        mRawSource = std::make_shared<YUVSource>();
    }

    mRawSource->setDataSource(file, mWidth, mHeight, mBitDepth, mPixelFormat, mColorAspect);
    mIsRunning = true;
    int fps = (mFPS > 0) ? mFPS : 25;
    auto frameDuration = std::chrono::milliseconds(1000 / fps);

    mRenderThread = std::thread([this, frameDuration]() {
        while (mIsRunning) {
            auto nextFrameTime = std::chrono::steady_clock::now() + frameDuration;
            auto frame = mRawSource->getBuffer();
            if (frame) {
                QTDebug("EncoderTabViewModel", "Frame received");
                QMetaObject::invokeMethod(mVideoWidget, "bindBuffer",
                                          Qt::QueuedConnection,
                                          Q_ARG(std::shared_ptr<Qv2Buffer>, frame));
                emit playing(mRawSource->getCurrentFrame(), mRawSource->getTotalFrame());
                // mEncoder->encodeFrame(frame);
            } else {
                QTInfo("EncoderTabViewModel", "EOS received");
                mIsRunning = false;
                emit finished();
                break;
            }
            std::this_thread::sleep_until(nextFrameTime);
        }
    });
}

void EncoderTabViewModel::stop() {
    mIsRunning = false;
    mRawSource.reset();
    if (mRenderThread.joinable()) {
        mRenderThread.join();
    }
}

void EncoderTabViewModel::setPixelFormat(string value) {
    mPixelFormat = getValue(PixelFormatMap, value, QV2_CF_UNKNOWN);
}

void EncoderTabViewModel::setProfile(string value) {
    mProfile = getValue(ProfileMap, value, static_cast<Qv2APVProfile>(0));
}

void EncoderTabViewModel::setLevel(int value) {
    int finalLevelValue = (mBand << 8) | (value & 0xFF);
    mLevel = static_cast<Qv2APVLevel>(finalLevelValue);
}

void EncoderTabViewModel::setFamily(string value) {
    mFamily = getValue(FamilyMap, value, static_cast<Qv2APVFamily>(0));
}

void EncoderTabViewModel::setPrimaries(string value) {
    mColorAspect.primaries = getValue(PrimariesMap, value, QV2_CP_UNKNOWN);
}

void EncoderTabViewModel::setTransfer(string value) {
    mColorAspect.transfer = getValue(TransferMap, value, QV2_CT_UNKNOWN);
}

void EncoderTabViewModel::setMatrix(string value) {
    mColorAspect.matrix = getValue(MatrixMap, value, QV2_CM_UNKNOWN);
}

void EncoderTabViewModel::setRange(string value) {
    mColorAspect.range = (value == "full") ? QV2_CR_FULL : QV2_CR_LIMITED;
}