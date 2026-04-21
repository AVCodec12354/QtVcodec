#include "Qv2ComponentFactory.h"
#include "oapv.h"
#include <cstdio>
#include <vector>
#include <memory>
#include <cassert>

#define LOG_TAG "testMain"

class TestListener : public Qv2Component::Listener {
public:
    void onWorkDone(std::weak_ptr<Qv2Component> component,
                    std::vector<std::unique_ptr<Qv2Work>> workItems) override {
        printf("  [Listener] onWorkDone: processed %zu items\n", workItems.size());
        for (auto& item : workItems) {
            if (item->result == 0 && item->output) {
                auto out = static_cast<Qv2Buffer1D*>(item->output.get());
                printf("    - Encoded bitstream size: %zu bytes\n", out->getSize());
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

    // 3. Queue Work (Simulate 1 frame encoding)
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

        auto input = std::make_shared<Qv2Buffer2D>(width, height, OAPV_CF_YCBCR422, bitDepth);
        input->setPlane(PLANE_Y, reinterpret_cast<uint8_t*>(planeY.data()), yStride, height);
        input->setPlane(PLANE_U, reinterpret_cast<uint8_t*>(planeU.data()), cStride, height);
        input->setPlane(PLANE_V, reinterpret_cast<uint8_t*>(planeV.data()), cStride, height);
        item->input = input;

        item->output = std::make_shared<Qv2Buffer1D>(outData.data(), 0, outData.size());

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
    testComponent(Qv2ComponentFactory::ENCODER_APV, "ENCODER_APV");
    printf("All tests completed successfully!\n");
    return 0;
}
