#ifndef ENCODERVIEWMODEL_H
#define ENCODERVIEWMODEL_H

#include <iostream>
#include <memory>
#include <vector>
#include <thread>

#include <Qv2Component.h>
#include <Qv2ComponentFactory.h>
#include <Qv2Source.h>
#include <VideoGLWidget.h>

using namespace std;

class EncoderTabViewModel {
public:
    EncoderTabViewModel(VideoGLWidget *glWidget);
    ~EncoderTabViewModel();

    void start(const std::string &file);
    void stop();

    // Basic Settings:
    void setWidth(int value);
    void setHeight(int value);
    void setFPS(int value);
    void setBitDepth(int value);
    void setPixelFormat(string value);
    // Bitrate and Quality
    void enableBitrateABR(bool value);
    void setQuantizationParameters(int value);
    void setProfile(string value);
    void setLevel(int value);
    void setFamily(string value);
    void setBand(int value);
    // Optimize
    void setMaxCU(int value);
    void setSpeedCU(int value);
    void setWidthOfTile(int value);
    void setHeightOfTile(int value);
    // Color Metadata
    void setPrimaries(string value);
    void setTransfer(string value);
    void setMatrix(string value);
    void setRange(string value);
    void setMasteringDisplay(int value);
    void setContentLightLevel(int value);

private:
    std::thread mRenderThread;
    std::atomic<bool> mIsRunning{false};
    std::shared_ptr<Qv2Source> rawSource;
    VideoGLWidget* videoWidget;

    int mWidth, mHeight, mFPS, mBitDepth, mPixelFormat;
};

#endif // ENCODERVIEWMODEL_H