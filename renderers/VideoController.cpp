#include "VideoController.h"

VideoController::VideoController(QObject *parent)
        : QObject(parent) {
    m_widget = new VideoGLWidget();
    m_extractor = new Y4MExtractor();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &VideoController::onTimerTick);
}

VideoController::~VideoController() {
    stop();
    delete m_extractor;
    delete m_timer;
}

void VideoController::setListener(Listener listener) {
    m_listener = listener
}

bool VideoController::loadVideo(const QString &filePath) {
    m_extractor->setFile(filePath.toStdString());
    return true;
}

void VideoController::play() {
    QTDebug("VideoController", "Play video");
    if (!m_timer->isActive()) {
        int num = m_extractor->getY4MParam().fps_num;
        int den = m_extractor->getY4MParam().fps_den;
        int fps = num / den;
        m_timer->start(fps);
    }
}

void VideoController::pause() {
    QTDebug("VideoController", "Pause video");
    m_timer->stop();
}

void VideoController::stop() {
    QTDebug("VideoController", "Stopped video");
    m_timer->stop();
    m_listener.onFinished();
}

void VideoController::onTimerTick() {
    oapv_imgb_t* buffer = m_extractor->getBuffer();
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
        m_widget->setFrameData(frameData, buffer->w[0], buffer->h[0]);
        imgb_release(buffer);
        m_listener.onPlaying();
    } else {
        stop();
    }
}