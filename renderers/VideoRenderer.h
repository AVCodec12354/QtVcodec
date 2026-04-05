#ifndef VideoRenderer_H
#define VideoRenderer_H

#include <QObject>
#include <QTimer>
#include <QString>
#include "../readers/Y4MExtractor.h"
#include "VideoGLWidget.h"
#include "QTLogger.h"

class VideoRenderer : public QObject {
    Q_OBJECT
public:
    class Listener {
    public:
        virtual ~Listener() {}
        virtual void onPlaying(long currentFrame, long totalFrame) = 0;
        virtual void onFinished() = 0;
    };

    explicit VideoRenderer(VideoGLWidget *glWidget, QObject *parent = nullptr);
    ~VideoRenderer();

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