#include "Qv2ComponentFactory.h"
#include "Qv2Buffer.h"
#include "oapv.h"
#include <cstdio>
#include <vector>
#include <memory>
#include <cassert>

#define LOG_TAG "testMain"

/**
 * @brief Kiểm tra việc tạo Qv2Buffer theo kiến trúc mới (Block-based)
 */
void testBufferCreation() {
    printf("--- Testing Qv2Buffer Creation ---\n");

    // 1. Test Linear Buffer (dùng cho Bitstream)
    printf("[1] Creating Linear Buffer...\n");
    std::vector<uint8_t> rawData(1024, 0xAB);
    auto block1d = std::make_shared<Qv2Block1D>(rawData.data(), 0, rawData.size());
    auto linearBuf = Qv2Buffer::CreateLinearBuffer(block1d);
    
    assert(linearBuf->type() == Qv2Buffer::LINEAR);
    assert(linearBuf->linearBlocks().size() == 1);
    assert(linearBuf->linearBlocks()[0]->capacity() == 1024);
    printf("    Linear Buffer created successfully. Capacity: %zu\n", linearBuf->linearBlocks()[0]->capacity());

    // 2. Test Graphic Buffer (dùng cho YUV/Raw frames)
    printf("[2] Creating Graphic Buffer (YUV422)...\n");
    uint32_t w = 128, h = 128;
    auto block2d = std::make_shared<Qv2Block2D>(w, h, OAPV_CF_YCBCR422, 10);
    
    // Giả lập cấp phát plane
    std::vector<uint16_t> yPlane(w * h, 512);
    block2d->setPlane(PLANE_Y, reinterpret_cast<uint8_t*>(yPlane.data()), w * 2, h);
    
    auto graphicBuf = Qv2Buffer::CreateGraphicBuffer(block2d);
    
    assert(graphicBuf->type() == Qv2Buffer::GRAPHIC);
    assert(graphicBuf->graphicBlocks().size() == 1);
    assert(graphicBuf->graphicBlocks()[0]->width() == 128);
    assert(graphicBuf->graphicBlocks()[0]->numPlanes() == 1);
    printf("    Graphic Buffer created successfully. Size: %dx%d, Planes: %d\n", 
           graphicBuf->graphicBlocks()[0]->width(), 
           graphicBuf->graphicBlocks()[0]->height(),
           graphicBuf->graphicBlocks()[0]->numPlanes());

    printf("--- Buffer Creation Test PASSED ---\n\n");
}

class TestListener : public Qv2Component::Listener {
public:
    void onWorkDone(std::weak_ptr<Qv2Component> component,
                    std::vector<std::unique_ptr<Qv2Work>> workItems) override {
        printf("  [Listener] onWorkDone: processed %zu items\n", workItems.size());
        for (auto& item : workItems) {
            if (item->result == 0 && item->output && item->output->type() == Qv2Buffer::LINEAR) {
                if (!item->output->linearBlocks().empty()) {
                    auto outBlock = item->output->linearBlocks()[0];
                    printf("    - Encoded bitstream size: %zu bytes\n", outBlock->size());
                }
            }
        }
    }

    void onError(std::weak_ptr<Qv2Component> component,
                 Qv2Status error) override {
        printf("  [Listener] onError: %d\n", static_cast<int>(error));
    }

    void onStateChanged(std::weak_ptr<Qv2Component> component,
                        Qv2Component::State newState) override {
        printf("  [Listener] onStateChanged to: %s\n",
               Qv2Component::stateToString(newState).c_str());
    }
};

void testComponent(Qv2ComponentFactory::ComponentType type,
                   const std::string& typeName) {
    printf("Testing Component Type: %s\n", typeName.c_str());

    auto component = Qv2ComponentFactory::createByType(type);
    assert(component != nullptr);

    TestListener listener;
    component->setListener(&listener);

    // 1. Configure
    std::vector<Qv2Param*> params;
    Qv2VideoSizeInput size;
    Qv2FrameRateInput fps;
    Qv2BitrateSetting bitrate;
    Qv2BitDepthInput depth;
    Qv2ColorFormatInput color;
    Qv2QPInput qp;

    if (type == Qv2ComponentFactory::ENCODER_APV) {
        size.mWidth = 1920; size.mHeight = 1080; params.push_back(&size);
        fps.mFps = 30.0f; params.push_back(&fps);
        bitrate.mBitrate = 10000000; params.push_back(&bitrate);
        depth.mBitDepth = 10; params.push_back(&depth);
        color.mColorFormat = OAPV_CF_YCBCR422; params.push_back(&color);
        qp.mQP = 32; params.push_back(&qp);
    }
    component->configure(params);

    // 2. Start
    component->start();

    // 3. Queue Work
    if (type == Qv2ComponentFactory::ENCODER_APV) {
        constexpr uint32_t width = 1920;
        constexpr uint32_t height = 1080;
        constexpr uint32_t bitDepth = 10;
        constexpr uint32_t chromaWidth = width / 2;
        const uint32_t yStride = width * sizeof(uint16_t);
        const uint32_t cStride = chromaWidth * sizeof(uint16_t);

        std::vector<uint16_t> planeY(width * height, 64);
        std::vector<uint16_t> planeU(chromaWidth * height, 512);
        std::vector<uint16_t> planeV(chromaWidth * height, 512);
        std::vector<uint8_t> outData(1024 * 1024);

        std::vector<std::unique_ptr<Qv2Work>> items;
        auto item = std::make_unique<Qv2Work>();

        // Cách tạo Input Buffer mới
        auto inputBlock = std::make_shared<Qv2Block2D>(width, height, OAPV_CF_YCBCR422, bitDepth);
        inputBlock->setPlane(PLANE_Y, reinterpret_cast<uint8_t*>(planeY.data()), yStride, height);
        inputBlock->setPlane(PLANE_U, reinterpret_cast<uint8_t*>(planeU.data()), cStride, height);
        inputBlock->setPlane(PLANE_V, reinterpret_cast<uint8_t*>(planeV.data()), cStride, height);
        item->input = Qv2Buffer::CreateGraphicBuffer(inputBlock);

        // Cách tạo Output Buffer mới
        auto outputBlock = std::make_shared<Qv2Block1D>(outData.data(), 0, outData.size());
        item->output = Qv2Buffer::CreateLinearBuffer(outputBlock);

        items.push_back(std::move(item));
        printf("  Queueing 1 frame for encoding...\n");
        component->queue(std::move(items));
    }

    // 4. Clean up
    component->stop();
    component->release();
    printf("Test %s FINISHED\n\n", typeName.c_str());
}

int main() {
    printf("=== Qv2Component Integration Tests ===\n");
    
    // Chạy test tạo buffer trước
    testBufferCreation();
    
    // Test tích hợp component
    testComponent(Qv2ComponentFactory::ENCODER_APV, "ENCODER_APV");
    
    printf("All tests completed successfully!\n");
    return 0;
}
