#ifndef QV2BUFFER_H
#define QV2BUFFER_H

#include <cstdint>
#include <cstddef>
#include <vector>

/**
 * @brief Buffer types: 1D for linear data (bitstream), 2D for image data (YUV)
 */
enum class Qv2BufferType {
    BUFFER_1D,
    BUFFER_2D
};

/**
 * @brief Unified Buffer class for both 1D (Linear) and 2D (Graphic) data.
 */
class Qv2Buffer {
public:
    struct Plane {
        uint8_t* addr;
        uint32_t stride;
        uint32_t width;
        uint32_t height;
    };

    virtual ~Qv2Buffer() = default;
    
    virtual Qv2BufferType type() const = 0;
    uint64_t timestamp = 0;
    uint32_t flags = 0;

    // 1D Interface
    virtual uint8_t* data() { return nullptr; }
    virtual size_t size() const { return 0; }
    virtual size_t capacity() const { return 0; }

    // 2D Interface
    virtual uint32_t width() const { return 0; }
    virtual uint32_t height() const { return 0; }
    virtual int format() const { return 0; }
    virtual int numPlanes() const { return 0; }
    virtual Plane plane(int /*index*/) const { return {nullptr, 0, 0, 0}; }
};

#endif // QV2BUFFER_H
