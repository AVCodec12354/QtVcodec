#ifndef VIDEOCONTROLLER_H
#define VIDEOCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QString>
#include "Y4MExtractor.h"
#include "VideoGLWidget.h"
#include "QTLogger.h"

class VideoController : public QObject {
    Q_OBJECT
public:
    class Listener {
    public:
        virtual ~Listener() {}
        virtual void onPlaying(long currentFrame, long totalFrame) = 0;
        virtual void onFinished() = 0;
    };

    explicit VideoController(QObject *parent = nullptr, VideoGLWidget *glWidget = nullptr);
    ~VideoController();

    VideoGLWidget* getVideoWidget() const { return m_widget; }

    void setListener(Listener *listener);
    bool loadVideo(const QString &filePath);
    void play();
    void pause();
    void stop();

private slots:
            void onTimerTick();

private:
    Listener *m_listener = nullptr;
    VideoGLWidget *m_widget = nullptr;
    Y4MExtractor *m_extractor = nullptr;
    QTimer *m_timer = nullptr;
    long m_currentFrame = 0;
};

#endif