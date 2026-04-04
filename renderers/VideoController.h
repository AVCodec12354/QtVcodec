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

    explicit VideoController(VideoGLWidget *glWidget, QObject *parent = nullptr);
    ~VideoController();

    void setListener(Listener *listener);
    bool loadVideo(const QString &filePath);
    void play();
    void pause();
    void stop();

private slots:
            void onTimerTick();

private:
    // Manage by UI, So don't need to use SmartPointer
    VideoGLWidget *mWidget = nullptr;
    Listener* mListener = nullptr;

    std::unique_ptr<Y4MExtractor> mExtractor{nullptr};
    std::unique_ptr<QTimer> mTimer{nullptr};
    long mCurrentFrame = 0;
};

#endif