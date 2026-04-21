#include "Qv2ComponentFactory.h"
#include <cstdio>
#include <vector>
#include <memory>
#include <cassert>

#define LOG_TAG "testMain"

class TestListener : public Qv2Component::Listener {
public:
    void onWorkDone(std::weak_ptr<Qv2Component> component,
                    std::vector<std::unique_ptr<Qv2Work>> workItems) override {
        printf("  [Listener] onWorkDone\n");
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

    printf("  Component created: %s (v%s)\n",
           component->getName().c_str(),
           component->getVersion().c_str());

    TestListener listener;
    component->setListener(&listener);

    printf("  Current State: %s\n", 
           Qv2Component::stateToString(component->getState()).c_str());
    assert(component->getState() == Qv2Component::INITIALIZED);

    printf("  Configuring...\n");
    std::vector<Qv2Param*> params;
    Qv2VideoSizeInput size;
    Qv2FrameRateInput fps;
    Qv2BitrateSetting bitrate;
    Qv2BitDepthInput depth;
    Qv2ColorFormatInput color;
    Qv2ProfileOutput profile;
    Qv2LevelOutput level;
    Qv2BandOutput band;
    Qv2QPInput qp;

    if (type == Qv2ComponentFactory::ENCODER_APV) {
        size.mWidth = 1920;
        size.mHeight = 1080;
        params.push_back(&size);

        fps.mFps = 30.0f;
        params.push_back(&fps);

        bitrate.mBitrate = 10000000;
        params.push_back(&bitrate);

        depth.mBitDepth = 10;
        params.push_back(&depth);

        color.mColorFormat = 12; // OAPV_CF_YCBCR422
        params.push_back(&color);

        profile.mProfile = 33;
        params.push_back(&profile);

        level.mLevel = 153;
        params.push_back(&level);

        band.mBand = 0;
        params.push_back(&band);

        qp.mQP = 32;
        params.push_back(&qp);
    }

    Qv2Status status = component->configure(params);
    printf("  Configure status: %d\n", static_cast<int>(status));

    printf("  Starting...\n");
    status = component->start();
    printf("  Start status: %d\n", static_cast<int>(status));

    printf("  Stopping...\n");
    status = component->stop();
    printf("  Stop status: %d\n", static_cast<int>(status));

    printf("  Releasing...\n");
    component->release();

    printf("Test %s PASSED\n\n", typeName.c_str());
}

int main() {
    printf("=== Qv2Component Unit Tests ===\n");

    try {
        testComponent(Qv2ComponentFactory::ENCODER_APV, "ENCODER_APV");
        testComponent(Qv2ComponentFactory::DECODER_APV, "DECODER_APV");
    } catch (const std::exception& e) {
        fprintf(stderr, "Test failed with exception: %s\n", e.what());
        return 1;
    }

    printf("All tests completed successfully!\n");
    return 0;
}
