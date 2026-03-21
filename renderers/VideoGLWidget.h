#ifndef VIDEOGLWIDGET_H
#define VIDEOGLWIDGET_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QFile>
#include <QTimer>
#include <QDebug>

class VideoGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
public:
    VideoGLWidget(QWidget *parent = nullptr) : QOpenGLWidget(parent) {}
    ~VideoGLWidget();
    void setFrameData(const QByteArray &data, int w, int h);
protected:
    void initializeGL() override;
    void paintGL() override;
private:
    void renderPlane(int index, const unsigned short* data, int w, int h);
private:
    QOpenGLShaderProgram mProgram;
    GLuint mTextures[3];
    QByteArray mData;
    int mWidth = 0, mHeight = 0;
};


#endif