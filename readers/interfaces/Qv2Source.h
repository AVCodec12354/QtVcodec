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
    void config(int width, int height) {
        this->width = width;
        this->height = height;
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
                  mColorFormat(QV2_CF_YCBCR420) {};
    virtual ~Qv2Source() {
        if (mFile.is_open()) {
            mFile.close();
        }
    };

    virtual void setDataSource(
            const std::string &filePath,
            int width,
            int height,
            int bitDepth = QV2_DEFAULT_BIT_DEPTH,
            Qv2ColorFormat colorFormat = QV2_CF_YCBCR420,
            Qv2ColorPrimaries colorPrimaries = QV2_CP_BT709,
            Qv2ColorTransfer colorTransfer = QV2_CT_BT709,
            Qv2ColorMatrix colorMatrix = QV2_CM_BT709,
            Qv2ColorRange colorRange = QV2_CR_FULL
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
        mColorPrimaries = colorPrimaries;
        mColorTransfer = colorTransfer;
        mColorMatrix = colorMatrix;
        mColorRange = colorRange;

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
    Qv2ColorPrimaries mColorPrimaries;
    Qv2ColorTransfer mColorTransfer;
    Qv2ColorMatrix mColorMatrix;
    Qv2ColorRange mColorRange;
};
