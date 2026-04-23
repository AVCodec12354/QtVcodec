#pragma once

#include <iostream>
#include <string>
#include <memory>

#include <Qv2Buffer.h>
#include <Qv2Constants.h>

class PlaneInfo {
private:
    int width;
    int height;
public:
    void config(int w, int h) {
        width = w;
        height = h;
    }

    int getWidth() { return width; }
    int getHeight() { return height; }
    int getSize() { return width * height; }
};

class Qv2Source {
public:
    Qv2Source() : mCurrentFrame(0),
                  mTotalFrame(0),
                  mFile(nullptr, &std::fclose) {};
    virtual ~Qv2Source() = default;

    void setDataSource(
            std::string filePath,
            int width,
            int height,
            int bitDepth = 8,
            ColorFormat colorFormat = QV2FormatYUV420Planar
    ) {
        mFile.reset(fopen(filePath.c_str(), "rb"));
        mWidth = width;
        mHeight = height;
        mBitDepth = bitDepth;
        mColorFormat = colorFormat;
        mCurrentFrame = 0;
        calculatePlaneSize();
        mTotalFrame = calculateTotalFrame();
    };

    virtual std::unique_ptr<Qv2Buffer> getBuffer() = 0;
    int64_t getCurrentFrame() { return mCurrentFrame; }
    int64_t getTotalFrame() { return mTotalFrame; }

protected:
    virtual int64_t calculateTotalFrame() = 0;
    virtual void calculatePlaneSize() = 0;

    int mNumOfPlane = 0;
    PlaneInfo mPlaneInfo[4];
    int mWidth, mHeight, mBitDepth;
    int64_t mCurrentFrame, mTotalFrame;
    std::unique_ptr<FILE, decltype(&std::fclose)> mFile;
    ColorFormat mColorFormat;
};
