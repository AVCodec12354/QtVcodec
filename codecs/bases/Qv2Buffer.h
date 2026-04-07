#ifndef QV2BUFFER_H
#define QV2BUFFER_H

#include <cstdint>
#include <cstddef>

/**
 * @brief Buffer types: 1D for linear data (bitstream), 2D for image data (YUV)
 */
enum class Qv2BufferType {
    BUFFER_1D,
    BUFFER_2D
};

/**
 * @brief Base class for all Qv2 Buffers
 */
class Qv2Buffer {
public:
    virtual ~Qv2Buffer() = default;
    virtual Qv2BufferType type() const = 0;

    uint64_t timestamp = 0; // PTS
    uint32_t flags = 0;
};

/**
 * @brief 1D Buffer for linear data like Bitstream
 */
class Qv2Buffer1D : public Qv2Buffer {
public:
    Qv2BufferType type() const override { return Qv2BufferType::BUFFER_1D; }

    virtual uint8_t* data() = 0;
    virtual size_t size() const = 0;
    virtual size_t capacity() const = 0;
};

/**
 * @brief 2D Buffer for image data like YUV
 */
class Qv2Buffer2D : public Qv2Buffer {
public:
    Qv2BufferType type() const override { return Qv2BufferType::BUFFER_2D; }

    struct Plane {
        uint8_t* addr;
        uint32_t stride;
        uint32_t width;
        uint32_t height;
    };

    virtual uint32_t width() const = 0;
    virtual uint32_t height() const = 0;
    virtual int format() const = 0;
    virtual int numPlanes() const = 0;
    virtual Plane plane(int index) const = 0;
};

#endif // QV2BUFFER_H
