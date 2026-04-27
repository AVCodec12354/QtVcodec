#include "TestUtils.h"
#include "Qv2ComponentFactory.h"

class Qv2ComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        mComponent = Qv2ComponentFactory::createByType(Qv2ComponentFactory::ENCODER_APV);
        ASSERT_NE(mComponent, nullptr);
    }
    std::shared_ptr<Qv2Component> mComponent;
};

TEST_F(Qv2ComponentTest, InitialState) {
    EXPECT_EQ(mComponent->getState(), Qv2Component::INITIALIZED);
    EXPECT_FALSE(mComponent->getName().empty());
    EXPECT_FALSE(mComponent->getVersion().empty());
}

TEST_F(Qv2ComponentTest, StateTransitions) {
    EXPECT_EQ(mComponent->getState(), Qv2Component::INITIALIZED);

    std::vector<Qv2Param*> params;
    auto sizeParam = std::make_shared<Qv2VideoSizeInput>();
    sizeParam->mWidth = 1920; sizeParam->mHeight = 1080;
    params.push_back(sizeParam.get());

    auto colorParam = std::make_shared<Qv2ColorFormatInput>();
    colorParam->mColorFormat = QV2_CF_YCBCR422_10LE;
    params.push_back(colorParam.get());

    auto fpsParam = std::make_shared<Qv2FrameRateInput>();
    fpsParam->mFps = 30.0f;
    params.push_back(fpsParam.get());

    auto depthParam = std::make_shared<Qv2BitDepthInput>();
    depthParam->mBitDepth = 10;
    params.push_back(depthParam.get());

    auto bitrateParam = std::make_shared<Qv2BitrateSetting>();
    bitrateParam->mBitrate = 10000000;
    params.push_back(bitrateParam.get());

    auto qpParam = std::make_shared<Qv2QPInput>();
    qpParam->mQP = 25;
    params.push_back(qpParam.get());

    auto plParam = std::make_shared<Qv2ProfileLevelInput>();
    plParam->mProfile = QV2_APV_PROFILE_422_10;
    plParam->mLevel = QV2_APV_LEVEL_5_1_BAND_0;
    params.push_back(plParam.get());

    auto familyParam = std::make_shared<Qv2APVFamilySetting>();
    familyParam->mFamily = QV2_APV_FAMILY_422_HQ;
    params.push_back(familyParam.get());

    EXPECT_EQ(mComponent->configure(params), QV2_OK);
    EXPECT_EQ(mComponent->getState(), Qv2Component::CONFIGURED);

    EXPECT_EQ(mComponent->start(), QV2_OK);
    EXPECT_EQ(mComponent->getState(), Qv2Component::RUNNING);

    EXPECT_EQ(mComponent->stop(), QV2_OK);
    EXPECT_EQ(mComponent->getState(), Qv2Component::STOPPED);

    mComponent->release();
    EXPECT_EQ(mComponent->getState(), Qv2Component::UNINITIALIZED);
}

TEST_F(Qv2ComponentTest, ParameterQuery) {
    std::vector<Qv2Param*> queryParams;
    Qv2Status status = mComponent->query(queryParams);
    EXPECT_EQ(status, QV2_OK);
}

TEST_F(Qv2ComponentTest, FlushOperation) {
    std::vector<Qv2Param*> params;
    auto sizeParam = std::make_shared<Qv2VideoSizeInput>();
    sizeParam->mWidth = 1920; sizeParam->mHeight = 1080;
    params.push_back(sizeParam.get());

    auto colorParam = std::make_shared<Qv2ColorFormatInput>();
    colorParam->mColorFormat = QV2_CF_YCBCR422_10LE;
    params.push_back(colorParam.get());

    auto fpsParam = std::make_shared<Qv2FrameRateInput>();
    fpsParam->mFps = 30.0f;
    params.push_back(fpsParam.get());

    auto depthParam = std::make_shared<Qv2BitDepthInput>();
    depthParam->mBitDepth = 10;
    params.push_back(depthParam.get());

    auto bitrateParam = std::make_shared<Qv2BitrateSetting>();
    bitrateParam->mBitrate = 10000000;
    params.push_back(bitrateParam.get());

    auto qpParam = std::make_shared<Qv2QPInput>();
    qpParam->mQP = 25;
    params.push_back(qpParam.get());

    ASSERT_EQ(mComponent->configure(params), QV2_OK);
    ASSERT_EQ(mComponent->start(), QV2_OK);
    EXPECT_EQ(mComponent->flush(), QV2_OK);
    EXPECT_EQ(mComponent->stop(), QV2_OK);
    mComponent->release();
}

class TestComponentListener : public Qv2Component::Listener {
public:
    void onWorkDone(std::weak_ptr<Qv2Component> component, std::vector<std::unique_ptr<Qv2Work>> workItems) override {
        workDoneCalled = true;
        receivedWorkItems = workItems.size();
    }
    void onError(std::weak_ptr<Qv2Component> component, Qv2Status error) override {
        errorCalled = true;
        lastError = error;
    }
    void onStateChanged(std::weak_ptr<Qv2Component> component, Qv2Component::State newState) override {
        stateChangedCalled = true;
        lastState = newState;
    }
    bool workDoneCalled = false;
    bool errorCalled = false;
    bool stateChangedCalled = false;
    size_t receivedWorkItems = 0;
    Qv2Status lastError = QV2_OK;
    Qv2Component::State lastState = Qv2Component::UNINITIALIZED;
};

TEST_F(Qv2ComponentTest, ListenerCallbacks) {
    TestComponentListener listener;
    mComponent->setListener(&listener);

    std::vector<Qv2Param*> params;
    auto sizeParam = std::make_shared<Qv2VideoSizeInput>();
    sizeParam->mWidth = 1920; sizeParam->mHeight = 1080;
    params.push_back(sizeParam.get());

    auto colorParam = std::make_shared<Qv2ColorFormatInput>();
    colorParam->mColorFormat = QV2_CF_YCBCR422_10LE;
    params.push_back(colorParam.get());

    auto fpsParam = std::make_shared<Qv2FrameRateInput>();
    fpsParam->mFps = 30.0f;
    params.push_back(fpsParam.get());

    auto depthParam = std::make_shared<Qv2BitDepthInput>();
    depthParam->mBitDepth = 10;
    params.push_back(depthParam.get());

    auto bitrateParam = std::make_shared<Qv2BitrateSetting>();
    bitrateParam->mBitrate = 10000000;
    params.push_back(bitrateParam.get());

    auto qpParam = std::make_shared<Qv2QPInput>();
    qpParam->mQP = 25;
    params.push_back(qpParam.get());

    ASSERT_EQ(mComponent->configure(params), QV2_OK);
    EXPECT_TRUE(listener.stateChangedCalled);
    EXPECT_EQ(listener.lastState, Qv2Component::CONFIGURED);

    listener.stateChangedCalled = false;
    ASSERT_EQ(mComponent->start(), QV2_OK);
    EXPECT_TRUE(listener.stateChangedCalled);
    EXPECT_EQ(listener.lastState, Qv2Component::RUNNING);

    listener.stateChangedCalled = false;
    ASSERT_EQ(mComponent->stop(), QV2_OK);
    EXPECT_TRUE(listener.stateChangedCalled);
    EXPECT_EQ(listener.lastState, Qv2Component::STOPPED);

    listener.stateChangedCalled = false;
    mComponent->release();
    EXPECT_TRUE(listener.stateChangedCalled);
    EXPECT_EQ(listener.lastState, Qv2Component::UNINITIALIZED);
}

TEST_F(Qv2ComponentTest, InvalidStateOperations) {
    EXPECT_NE(mComponent->start(), QV2_OK);
    EXPECT_NE(mComponent->stop(), QV2_OK);

    std::vector<Qv2Param*> params;
    auto sizeParam = std::make_shared<Qv2VideoSizeInput>();
    sizeParam->mWidth = 1920; sizeParam->mHeight = 1080;
    params.push_back(sizeParam.get());

    auto colorParam = std::make_shared<Qv2ColorFormatInput>();
    colorParam->mColorFormat = QV2_CF_YCBCR422_10LE;
    params.push_back(colorParam.get());

    auto fpsParam = std::make_shared<Qv2FrameRateInput>();
    fpsParam->mFps = 30.0f;
    params.push_back(fpsParam.get());

    auto depthParam = std::make_shared<Qv2BitDepthInput>();
    depthParam->mBitDepth = 10;
    params.push_back(depthParam.get());

    auto bitrateParam = std::make_shared<Qv2BitrateSetting>();
    bitrateParam->mBitrate = 10000000;
    params.push_back(bitrateParam.get());

    auto qpParam = std::make_shared<Qv2QPInput>();
    qpParam->mQP = 25;
    params.push_back(qpParam.get());

    ASSERT_EQ(mComponent->configure(params), QV2_OK);
    EXPECT_NE(mComponent->configure(params), QV2_OK);

    ASSERT_EQ(mComponent->start(), QV2_OK);
    EXPECT_NE(mComponent->configure(params), QV2_OK);

    EXPECT_EQ(mComponent->stop(), QV2_OK);
    mComponent->release();
}

TEST(ComponentFactoryTest, CreateByType) {
    auto encoder = Qv2ComponentFactory::createByType(Qv2ComponentFactory::ENCODER_APV);
    EXPECT_NE(encoder, nullptr);
    EXPECT_EQ(encoder->getState(), Qv2Component::INITIALIZED);
}

TEST(ComponentFactoryTest, CreateByName) {
    auto encoder = Qv2ComponentFactory::createByName("qv2.apv.encoder");
    EXPECT_NE(encoder, nullptr);

    auto decoder = Qv2ComponentFactory::createByName("qv2.apv.decoder");
    EXPECT_NE(decoder, nullptr);

    auto invalid = Qv2ComponentFactory::createByName("invalid.name");
    EXPECT_EQ(invalid, nullptr);
}
