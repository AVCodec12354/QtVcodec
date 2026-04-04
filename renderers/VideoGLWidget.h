#ifndef VIDEOGLWIDGET_H
#define VIDEOGLWIDGET_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QFile>
#include <QTimer>
#include <QDebug>
#include <QMutex>

#include "../readers/oapv_app_y4m.h"

class VideoGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
public:
    VideoGLWidget(QWidget *parent = nullptr) : QOpenGLWidget(parent) {}
    ~VideoGLWidget();
    void setFrameData(oapv_imgb_t* buffer);
protected:
    void initializeGL() override;
    void paintGL() override;
private:
    void renderPlane(int index, const unsigned short* data, int w, int h);
private:
    QMutex mMutex;
    QOpenGLShaderProgram mProgram;
    GLuint mTextures[3] = {0, 0, 0};
    oapv_imgb_t* mBuffer = nullptr;
    int mWidth = 0, mHeight = 0;
};


#endif