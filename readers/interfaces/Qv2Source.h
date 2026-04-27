#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <fstream>

#include <Qv2Buffer.h>
#include <Qv2Constants.h>

#define MAX_PLANE 4
#define QV2_DEFAULT_BIT_DEPTH 8

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
                  mWidth(0),
                  mHeight(0),
                  mBitDepth(QV2_DEFAULT_BIT_DEPTH),
                  mColorFormat(QV2FormatYUV420Planar) {};
    virtual ~Qv2Source() {
        if (mFile.is_open()) {
            mFile.close();
        }
    };

    virtual void setDataSource(
            std::string filePath,
            int width,
            int height,
            int bitDepth = QV2_DEFAULT_BIT_DEPTH,
            Qv2ColorFormat colorFormat = QV2FormatYUV420Planar
    ) {
        std::cout << "Open file: " << filePath << std::endl;
        mFile.open(filePath, std::ios::binary | std::ios::in);

        if (!mFile.is_open()) {
            std::cout << "Failed to open file: " << filePath << std::endl;
            return;
        }

        mWidth = width;
        mHeight = height;
        mBitDepth = bitDepth;
        mColorFormat = colorFormat;
        mCurrentFrame = 0;

        calculatePlaneSize();
        mTotalFrame = calculateTotalFrame();
    };

    virtual std::shared_ptr<Qv2Buffer> getBuffer() = 0;
    int64_t getCurrentFrame() const { return mCurrentFrame; }
    int64_t getTotalFrame() const { return mTotalFrame; }
    int getWidth() const { return mWidth; }
    int getHeight() const { return mHeight; }

protected:
    virtual int64_t calculateTotalFrame() = 0;
    virtual void calculatePlaneSize() = 0;

    int mNumOfPlane = 0;
    PlaneInfo mPlaneInfo[MAX_PLANE];
    int mWidth, mHeight, mBitDepth;
    int64_t mCurrentFrame, mTotalFrame;
    std::ifstream mFile;
    Qv2ColorFormat mColorFormat;
};
