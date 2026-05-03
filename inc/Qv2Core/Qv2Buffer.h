#ifndef QV2BUFFER_H
#define QV2BUFFER_H

#include <cstdint>
#include <cstddef>
#include <memory>
#include <vector>
#include <Qv2Constants.h>

enum Qv2NumPlane : uint32_t {
    PLANE_Y = 0,
    PLANE_U = 1,
    PLANE_V = 2,
    PLANE_R = 0,
    PLANE_G = 1,
    PLANE_B = 2,
    PLANE_A = 3,
    PLANE_UV = 1,
    MAX_NUM_PLANES = 4,
};


class Qv2Block1D {
public:
    Qv2Block1D(uint8_t* data, size_t size, size_t capacity)
        : mData(data), mSize(size), mCapacity(capacity) {}

    uint8_t* data() { return mData; }
    const uint8_t* data() const { return mData; }
    size_t size() const { return mSize; }
    size_t capacity() const { return mCapacity; }
    void setSize(size_t size) { mSize = size; }

private:
    uint8_t* mData;
    size_t mSize;
    size_t mCapacity;
};


class Qv2Block2D {
public:
    Qv2Block2D(uint32_t width, uint32_t height, uint32_t format, uint32_t bitDepth)
            : mWidth(width), mHeight(height), mFormat(format), mBitDepth(bitDepth), mNumPlanes(0) {
        for (uint32_t i = 0; i < MAX_NUM_PLANES; ++i) {
            mAddr[i] = nullptr;
            mStride[i] = 0;
            mElevation[i] = 0;
        }
    }

    virtual ~Qv2Block2D() {
        for (uint32_t i = 0; i < mNumPlanes; ++i) {
            if (mAddr[i]) {
                delete[] mAddr[i];
                mAddr[i] = nullptr;
            }
        }
    }

    uint32_t width() const { return mWidth; }
    uint32_t height() const { return mHeight; }
    uint32_t format() const { return mFormat; }
    uint32_t bitDepth() const { return mBitDepth; }
    uint32_t numPlanes() const { return mNumPlanes; }
    Qv2ColorPrimaries getColorPrimaries() const { return mColorPrimaries; }
    Qv2ColorTransfer getColorTransfer() const { return mColorTransfer; }
    Qv2ColorMatrix getColorMatrix() const { return mColorMatrix; }
    Qv2ColorRange getColorRange() const { return mColorRange; }

    uint8_t* addr(uint32_t index) const { return (index < MAX_NUM_PLANES) ? mAddr[index] : nullptr; }
    uint32_t stride(uint32_t index) const { return (index < MAX_NUM_PLANES) ? mStride[index] : 0; }
    uint32_t elevation(uint32_t index) const { return (index < MAX_NUM_PLANES) ? mElevation[index] : 0; }

    void setPlane(
            uint32_t index,
            uint8_t* addr,
            uint32_t stride,
            uint32_t elevation,
            Qv2ColorPrimaries colorPrimaries = QV2_CP_BT709,
            Qv2ColorTransfer colorTransfer = QV2_CT_BT709,
            Qv2ColorMatrix colorMatrix = QV2_CM_BT709,
            Qv2ColorRange colorRange = QV2_CR_FULL
    ) {
        if (index < MAX_NUM_PLANES) {
            mAddr[index] = addr;
            mStride[index] = stride;
            mElevation[index] = elevation;
            if (index >= mNumPlanes) mNumPlanes = index + 1;
        }
        mColorPrimaries = colorPrimaries;
        mColorTransfer = colorTransfer;
        mColorMatrix = colorMatrix;
        mColorRange = colorRange;
    }

private:
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mFormat;
    uint32_t mBitDepth;
    uint32_t mNumPlanes;
    uint8_t* mAddr[MAX_NUM_PLANES];
    uint32_t mStride[MAX_NUM_PLANES];
    uint32_t mElevation[MAX_NUM_PLANES];
    Qv2ColorPrimaries mColorPrimaries;
    Qv2ColorTransfer mColorTransfer;
    Qv2ColorMatrix mColorMatrix;
    Qv2ColorRange mColorRange;
};

/**
 * @brief Qv2Buffer: Lớp Container (giống C2Buffer).
 * Kết hợp Blocks dữ liệu với Metadata.
 */
class Qv2Buffer {
public:
    enum Type {
        EMPTY,
        LINEAR,
        GRAPHIC
    };


    static std::shared_ptr<Qv2Buffer> CreateLinearBuffer(std::shared_ptr<Qv2Block1D> block) {
        auto buffer = std::make_shared<Qv2Buffer>(LINEAR);
        buffer->mBlocks1D.push_back(block);
        return buffer;
    }

    static std::shared_ptr<Qv2Buffer> CreateGraphicBuffer(std::shared_ptr<Qv2Block2D> block) {
        auto buffer = std::make_shared<Qv2Buffer>(GRAPHIC);
        buffer->mBlocks2D.push_back(block);
        return buffer;
    }

    explicit Qv2Buffer(Type type) : mType(type) {}
    virtual ~Qv2Buffer() = default;

    Type type() const { return mType; }

    const std::vector<std::shared_ptr<Qv2Block1D>>& linearBlocks() const { return mBlocks1D; }
    const std::vector<std::shared_ptr<Qv2Block2D>>& graphicBlocks() const { return mBlocks2D; }

private:
    Type mType;
    std::vector<std::shared_ptr<Qv2Block1D>> mBlocks1D;
    std::vector<std::shared_ptr<Qv2Block2D>> mBlocks2D;
};

#endif // QV2BUFFER_H
