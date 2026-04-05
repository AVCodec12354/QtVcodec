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
    if (mWidget) delete mWidget;
    if (mListener) delete mListener;
}

void VideoRenderer::setListener(Listener *listener) {
    mListener = listener;
}

void VideoRenderer::setInputPath(const QString &filePath) {
    mExtractor->setFile(filePath.toStdString());
    updateGLWidgetSize();
}

void VideoRenderer::updateGLWidgetSize() {
    auto params = mExtractor->getY4MParam();
    if (mWidget && params.w > 0 && params.h > 0) {
        mWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        float ratioW = (float) MAX_LAYOUT_W / params.w;
        float ratioH = (float) MAX_LAYOUT_H / params.h;
        float scale = qMin(1.0f, qMin(ratioW, ratioH));

        int finalW = static_cast<int>(params.w * scale);
        int finalH = static_cast<int>(params.h * scale);

        mWidget->setFixedSize(finalW, finalH);
        mWidget->updateGeometry();
    }
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