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
        printf("  [Listener] onStateChanged to: %d\n",
               static_cast<int>(newState));
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

    assert(component->getState() == Qv2Component::INITIALIZED);

    printf("  Configuring...\n");
    std::vector<Qv2Param*> params;
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
//        testComponent(Qv2ComponentFactory::DECODER_APV, "DECODER_APV");
    } catch (const std::exception& e) {
        fprintf(stderr, "Test failed with exception: %s\n", e.what());
        return 1;
    }

    printf("All tests completed successfully!\n");
    return 0;
}
