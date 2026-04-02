#include "VideoController.h"

VideoController::VideoController(QObject *parent, VideoGLWidget *glWidget)
        : QObject(parent) {
    m_widget = glWidget;
    m_extractor = new Y4MExtractor();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &VideoController::onTimerTick);
}

VideoController::~VideoController() {
    stop();
    delete m_listener;
    delete m_extractor;
    delete m_timer;
}

void VideoController::setListener(Listener *listener) {
    m_listener = listener;
}

bool VideoController::loadVideo(const QString &filePath) {
    m_extractor->setFile(filePath.toStdString());
    auto params = m_extractor->getY4MParam();
    int vW = params.w;
    int vH = params.h;

    if (m_widget && vW > 0 && vH > 0) {
        m_widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        const int MAX_LAYOUT_W = 1280;
        const int MAX_LAYOUT_H = 720;

        float ratioW = (float) MAX_LAYOUT_W / vW;
        float ratioH = (float) MAX_LAYOUT_H / vH;
        float scale = qMin(1.0f, qMin(ratioW, ratioH));

        int finalW = static_cast<int>(vW * scale);
        int finalH = static_cast<int>(vH * scale);

        m_widget->setFixedSize(finalW, finalH);
        m_widget->updateGeometry();
    }
    return true;
}

void VideoController::play() {
    QTDebug("VideoController", "Play video");
    if (!m_timer->isActive()) {
        int num = m_extractor->getY4MParam().fps_num;
        int den = m_extractor->getY4MParam().fps_den;
        int fps = num / den;
        int time = 1000 / fps;
        m_timer->start(time);
    }
}

void VideoController::pause() {
    QTDebug("VideoController", "Pause video");
    m_timer->stop();
}

void VideoController::stop() {
    QTDebug("VideoController", "Stopped video");
    m_currentFrame = 0;
    m_timer->stop();
    if (m_listener) m_listener->onFinished();
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
        if (m_listener) m_listener->onPlaying(m_currentFrame++, m_extractor->getTotalFrame());
    } else {
        stop();
    }
}