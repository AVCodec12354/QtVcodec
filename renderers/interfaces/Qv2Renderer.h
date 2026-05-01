#pragma once

#include <iostream>
#include <algorithm>
#include <memory>

#include <Qv2Buffer.h>
#include <Qv2Constants.h>
#include <QPointer>
#include <QOpenGLWidget>

static constexpr int MAX_LAYOUT_W = 1280;
static constexpr int MAX_LAYOUT_H = 720;

class Qv2Renderer {
public:
    virtual ~Qv2Renderer() = default;
    virtual void renderFrame(std::shared_ptr<Qv2Buffer> buffer) = 0;

    void setSurfaceView(QOpenGLWidget* widget) {
        std::cout << __FUNCTION__ << std::endl;
        mWidget = widget;
    }
protected:
    void updateSurfaceSize(int width, int height) {
        std::cout << __FUNCTION__ << width << "x" << height << std::endl;
        if (!mWidget.isNull() && width > 0 && height > 0 &&
            (mWidget->width() != width || mWidget->height() != height)
        ) {
            mWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

            float ratioW = (float) MAX_LAYOUT_W / width;
            float ratioH = (float) MAX_LAYOUT_H / height;
            float scale = std::min(1.0f, std::min(ratioW, ratioH));

            int finalW = static_cast<int>(width * scale);
            int finalH = static_cast<int>(height * scale);
            mWidget->setFixedSize(finalW, finalH);
            mWidget->updateGeometry();
        }
    }
private:
    QPointer<QOpenGLWidget> mWidget = nullptr;
};