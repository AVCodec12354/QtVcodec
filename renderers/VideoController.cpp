#include "VideoController.h"

const int MAX_LAYOUT_W = 1280;
const int MAX_LAYOUT_H = 720;

VideoController::VideoController(VideoGLWidget *glWidget, QObject *parent)
        : QObject(parent)
        , mExtractor(std::make_unique<Y4MExtractor>())
        , mTimer(std::make_unique<QTimer>()) {
    mWidget = glWidget;
    connect(mTimer.get(), &QTimer::timeout, this, &VideoController::onTimerTick);
}

VideoController::~VideoController() {
    stop();
}

void VideoController::setListener(Listener *listener) {
    mListener = listener;
}

bool VideoController::loadVideo(const QString &filePath) {
    mExtractor->setFile(filePath.toStdString());
    auto params = mExtractor->getY4MParam();
    int vW = params.w;
    int vH = params.h;

    if (mWidget && vW > 0 && vH > 0) {
        mWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        float ratioW = (float) MAX_LAYOUT_W / vW;
        float ratioH = (float) MAX_LAYOUT_H / vH;
        float scale = qMin(1.0f, qMin(ratioW, ratioH));

        int finalW = static_cast<int>(vW * scale);
        int finalH = static_cast<int>(vH * scale);

        mWidget->setFixedSize(finalW, finalH);
        mWidget->updateGeometry();
    }
    return true;
}

void VideoController::play() {
    QTDebug("VideoController", "Play video");
    if (!mTimer->isActive()) {
        int num = mExtractor->getY4MParam().fps_num;
        int den = mExtractor->getY4MParam().fps_den;
        int fps = num / den;
        int time = 1000 / fps;
        mTimer->start(time > 0 ? time : 33);
    }
}

void VideoController::pause() {
    QTDebug("VideoController", "Pause video");
    mTimer->stop();
}

void VideoController::stop() {
    QTDebug("VideoController", "Stopped video");
    mCurrentFrame = 0;
    mTimer->stop();
    if (mListener) mListener->onFinished();
}

void VideoController::onTimerTick() {
    oapv_imgb_t* buffer = mExtractor->getBuffer();
    if (buffer != nullptr) {
        int totalSize = 0;
        for (int i = 0; i < 3; ++i) {
            totalSize += buffer->w[i] * buffer->h[i] * sizeof(unsigned short);
        }

        QByteArray frameData;
        frameData.resize(totalSize);
        unsigned short* destPtr = reinterpret_cast<unsigned short*>(frameData.data());

        for (int i = 0; i < 3; ++i) {
            unsigned char* srcPlane = reinterpret_cast<unsigned char*>(buffer->a[i]);
            int width = buffer->w[i];
            int height = buffer->h[i];
            int stride = buffer->s[i];

            for (int y = 0; y < height; ++y) {
                unsigned short* srcRow = reinterpret_cast<unsigned short*>(srcPlane + (y * stride));
                memcpy(destPtr, srcRow, width * sizeof(unsigned short));
                destPtr += width;
            }
        }
        if (mWidget) {
            mWidget->setFrameData(frameData, buffer->w[0], buffer->h[0]);

            if (mListener) {
                QTDebug("VideoController", "Current frame: " + QString::number(mCurrentFrame) +
                                           ", Total frame: " + QString::number(mExtractor->getTotalFrame()));
                mListener->onPlaying(mCurrentFrame++, mExtractor->getTotalFrame());
            }
        } else {
            QTError("VideoController", "Widget is null");
        }
    } else {
        stop();
    }
    imgb_release(buffer);
}