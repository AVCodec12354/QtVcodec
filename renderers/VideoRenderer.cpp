#include "VideoRenderer.h"

const int MAX_LAYOUT_W = 1280;
const int MAX_LAYOUT_H = 720;

VideoRenderer::VideoRenderer(VideoGLWidget *glWidget, QObject *parent)
        : QObject(parent)
        , mExtractor(std::make_unique<Y4MExtractor>())
        , mTimer(std::make_unique<QTimer>()) {
    mWidget = glWidget;
    connect(mTimer.get(), &QTimer::timeout, this, &VideoRenderer::onTimerTick);
}

VideoRenderer::~VideoRenderer() {
    stop();
}

void VideoRenderer::setListener(Listener *listener) {
    mListener = listener;
}

bool VideoRenderer::loadVideo(const QString &filePath) {
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

void VideoRenderer::play() {
    QTDebug("VideoRenderer", "Play video");
    if (!mTimer->isActive()) {
        int num = mExtractor->getY4MParam().fps_num;
        int den = mExtractor->getY4MParam().fps_den;
        int fps = num / den;
        int time = 1000 / fps;
        mTimer->start(time > 0 ? time : 33);
    }
}

void VideoRenderer::pause() {
    QTDebug("VideoRenderer", "Pause video");
    mTimer->stop();
}

void VideoRenderer::stop() {
    QTDebug("VideoRenderer", "Stopped video");
    mCurrentFrame = 0;
    mTimer->stop();
    if (mListener) mListener->onFinished();
}

void VideoRenderer::onTimerTick() {
    oapv_imgb_t* buffer = mExtractor->getBuffer();
    if (buffer != nullptr) {
        if (mWidget) {
            mWidget->setFrameData(buffer);
            if (mListener) {
                QTDebug("VideoRenderer", "Current frame: " + QString::number(mCurrentFrame) +
                                           ", Total frame: " + QString::number(mExtractor->getTotalFrame()));
                mListener->onPlaying(mCurrentFrame++, mExtractor->getTotalFrame());
            }
        } else {
            QTError("VideoRenderer", "Widget is null");
        }
        imgb_release(buffer);
    } else {
        stop();
    }
}