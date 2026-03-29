#ifndef VIDEOCONTROLLER_H
#define VIDEOCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QString>
#include "Y4MExtractor.h"
#include "VideoGLWidget.h"

class VideoController : public QObject {
    Q_OBJECT
public:
    explicit VideoController(QObject *parent = nullptr);
    ~VideoController();

    VideoGLWidget* getVideoWidget() const { return m_widget; }

    bool loadVideo(const QString &filePath);
    void play();
    void pause();
    void stop();

private slots:
            void onTimerTick();

private:
    VideoGLWidget *m_widget;
    Y4MExtractor *m_extractor;
    QTimer *m_timer;
};

#endif