#ifndef VIDEOGLWIDGET_H
#define VIDEOGLWIDGET_H

#include <memory>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <Qv2Buffer.h>
#include <Qv2Renderer.h>
#include <VideoRenderer.h>

class VideoGLWidget : public QOpenGLWidget, public QOpenGLFunctions {
    Q_OBJECT
public:
    explicit VideoGLWidget(QWidget *parent = nullptr) : QOpenGLWidget(parent) {
        std::cout << __FUNCTION__ << std::endl;
    }
    ~VideoGLWidget() {
        std::cout << __FUNCTION__ << std::endl;
        makeCurrent();
        mRenderer.reset();
        doneCurrent();
    }

    void bindBuffer(std::shared_ptr<Qv2Buffer> buffer) {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mCurrentBuffer = buffer;
        }
        std::cout << __FUNCTION__ << std::endl;
        QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
    }
protected:
    void initializeGL() override {
        std::cout << __FUNCTION__ << std::endl;
        initializeOpenGLFunctions();
        mRenderer = std::make_unique<VideoRenderer>();
        mRenderer->setSurfaceView(this);
    }

    void paintGL() override {
        std::shared_ptr<Qv2Buffer> frameToRender;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            frameToRender = mCurrentBuffer;
        }
        makeCurrent();
        std::cout << __FUNCTION__ << std::endl;
        if (mCurrentBuffer && mRenderer) {
            mRenderer->renderFrame(frameToRender);
        }
    }

    void resizeGL(int w, int h) override {
        std::cout << __FUNCTION__ << std::endl;
        glViewport(0, 0, w, h);
    }
private:
    std::unique_ptr<Qv2Renderer> mRenderer;
    std::shared_ptr<Qv2Buffer> mCurrentBuffer;
    std::mutex mMutex;
};

#endif