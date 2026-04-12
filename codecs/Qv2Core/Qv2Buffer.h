#ifndef QV2BUFFER_H
#define QV2BUFFER_H

#include <cstdint>
#include <cstddef>

enum class Qv2BufferType {
    BUFFER_1D,
    BUFFER_2D
};

class Qv2Buffer {
public:
    virtual ~Qv2Buffer() = default;
    virtual Qv2BufferType getType() const = 0;

    uint64_t mTimestamp = 0;
    uint32_t mFlags = 0;
};

class Qv2Buffer1D : public Qv2Buffer {
public:
    Qv2BufferType getType() const override { return Qv2BufferType::BUFFER_1D; }

    Qv2Buffer1D(uint8_t* data, size_t size, size_t capacity)
        : mData(data), mSize(size), mCapacity(capacity) {}

    uint8_t* getData() { return mData; }
    size_t getSize() const { return mSize; }
    size_t getCapacity() const { return mCapacity; }
    
    void setSize(size_t size) { mSize = size; }

private:
    uint8_t* mData;
    size_t mSize;
    size_t mCapacity;
};

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

class Qv2Buffer2D : public Qv2Buffer {
public:
    Qv2BufferType getType() const override { return Qv2BufferType::BUFFER_2D; }

    Qv2Buffer2D(uint32_t width, uint32_t height, uint32_t format, uint32_t bitDepth)
        : mWidth(width), mHeight(height), mFormat(format), mBitDepth(bitDepth), mNumPlanes(0) {
        for (uint32_t i = 0; i < MAX_NUM_PLANES; ++i) {
            mAddr[i] = nullptr;
            mStride[i] = 0;
            mElevation[i] = 0;
        }
    }

    uint32_t getWidth() const { return mWidth; }
    uint32_t getHeight() const { return mHeight; }
    uint32_t getBitDepth() const { return mBitDepth; }
    uint32_t getFormat() const { return mFormat; }
    uint32_t getNumPlanes() const { return mNumPlanes; }

    uint8_t* getAddr(uint32_t index) const { return (index < MAX_NUM_PLANES) ? mAddr[index] : nullptr; }
    uint32_t getStride(uint32_t index) const { return (index < MAX_NUM_PLANES) ? mStride[index] : 0; }
    uint32_t getElevation(uint32_t index) const { return (index < MAX_NUM_PLANES) ? mElevation[index] : 0; }

    void setPlane(uint32_t index, uint8_t* addr, uint32_t stride, uint32_t elevation) {
        if (index < MAX_NUM_PLANES) {
            mAddr[index] = addr;
            mStride[index] = stride;
            mElevation[index] = elevation;
            if (index >= mNumPlanes) {
                mNumPlanes = index + 1;
            }
        }
    }

private:
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mBitDepth;
    uint32_t mStride[MAX_NUM_PLANES];
    uint32_t mElevation[MAX_NUM_PLANES];
    uint32_t mFormat;
    uint32_t mNumPlanes;
    uint8_t* mAddr[MAX_NUM_PLANES];
};

#endif // QV2BUFFER_H
