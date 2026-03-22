#ifndef ENCODERVIEWMODEL_H
#define ENCODERVIEWMODEL_H

#include <iostream>

using namespace std;

class EncoderViewModel {
public:
    EncoderViewModel();

    void start();
    void stop();
    // Basic Settings:
    void setWidth(int value);
    void setHeight(int value);
    void setFPS(int value);
    void setBitDepth(int value);
    void setColorSpace(string value);
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
};

#endif // ENCODERVIEWMODEL_H