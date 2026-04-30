#include "TestUtils.h"
#include "Qv2ComponentFactory.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "Qv2EncoderTest"

static const std::string YuvInputFolder = "YUVTests/input";
static const std::string YuvOutputFolder = "YUVTests/output";

static const std::string Y4mInputFolder = "Y4MTests/input";
static const std::string Y4mOutputFolder = "Y4MTests/output";

class Qv2EncoderTestP : public ::testing::TestWithParam<TestParam> {
protected:
    void SetUp() override {
        mComponent = Qv2ComponentFactory::createByType(Qv2ComponentFactory::ENCODER_APV);
        ASSERT_NE(mComponent, nullptr);
    }
    std::shared_ptr<Qv2Component> mComponent;
};

TEST_P(Qv2EncoderTestP, RunYuvEncode) {
    const TestParam& p = GetParam();
    std::string fullInputPath = YuvInputFolder + p.inputPath;
    QV2_LOGI("Running encode for: %s", fullInputPath.c_str());

    FILE* fpIn = fopen(fullInputPath.c_str(), "rb");
    ASSERT_NE(fpIn, nullptr) << "Failed to open input file: " << fullInputPath;

    std::string fullOutputPath = YuvOutputFolder + p.outputPath;
    FILE* fpOut = fopen(fullOutputPath.c_str(), "wb");
    TestListener listener(fpOut);
    mComponent->setListener(&listener);

    int w = p.width, h = p.height, fmt = p.format, bitDepth = p.depth, numOfFrame = p.numOfFrame;
    float fpsValue = static_cast<float>(p.fps);

    std::vector<Qv2Param*> params;
    auto sizeParam = std::make_shared<Qv2VideoSizeInput>();
    sizeParam->mWidth = w; sizeParam->mHeight = h;
    params.push_back(sizeParam.get());

    auto colorParam = std::make_shared<Qv2ColorFormatInput>();
    colorParam->mColorFormat = fmt;
    params.push_back(colorParam.get());

    auto fpsParam = std::make_shared<Qv2FrameRateInput>();
    fpsParam->mFps = fpsValue;
    params.push_back(fpsParam.get());

    auto depthParam = std::make_shared<Qv2BitDepthInput>();
    depthParam->mBitDepth = bitDepth;
    params.push_back(depthParam.get());

    auto bitrateParam = std::make_shared<Qv2BitrateSetting>();
    bitrateParam->mBitrate = 100000000;
    params.push_back(bitrateParam.get());

    auto qpParam = std::make_shared<Qv2QPInput>();
    qpParam->mQP = 25;
    params.push_back(qpParam.get());

    ASSERT_EQ(mComponent->configure(params), QV2_OK);
    mComponent->enableRecon(true);
    ASSERT_EQ(mComponent->start(), QV2_OK);

    int cs = OAPV_CS_SET(toOapvFmt(fmt), bitDepth, 0);
    uint64_t intervalUs = static_cast<uint64_t>(1000000.0f / fpsValue);
    uint64_t curUs = 0;

    for (int i = 0; i < numOfFrame; ++i) {
        oapv_imgb_t* imgBuf = imgb_create(w, h, cs);
        if (imgb_read(fpIn, imgBuf, w, h, 0) < 0) {
            imgb_release(imgBuf);
            break;
        }

        auto item = std::make_unique<Qv2Work>();
        item->timestamp = curUs;
        curUs += intervalUs;

        auto inputBlock = std::make_shared<Qv2Block2D>(w, h, fmt, bitDepth);
        for (int plane = 0; plane < imgBuf->np; ++plane)
            inputBlock->setPlane(plane, (uint8_t*)imgBuf->a[plane], imgBuf->s[plane], imgBuf->h[plane]);
        item->input = Qv2Buffer::CreateGraphicBuffer(inputBlock);

        auto reconBlock = std::make_shared<Qv2Block2D>(w, h, fmt, bitDepth);
        std::vector<std::vector<uint8_t>> reconPlanes(imgBuf->np);
        for (int plane = 0; plane < imgBuf->np; ++plane) {
            reconPlanes[plane].resize(imgBuf->s[plane] * imgBuf->h[plane]);
            reconBlock->setPlane(plane, reconPlanes[plane].data(), imgBuf->s[plane], imgBuf->h[plane]);
        }
        item->recon = Qv2Buffer::CreateGraphicBuffer(reconBlock);

        std::vector<uint8_t> outData(w * h);
        item->output = Qv2Buffer::CreateLinearBuffer(std::make_shared<Qv2Block1D>(outData.data(), 0, outData.size()));

        std::vector<std::unique_ptr<Qv2Work>> items;
        items.push_back(std::move(item));
        mComponent->queue(std::move(items));

        imgb_release(imgBuf);
    }

    auto eosItem = std::make_unique<Qv2Work>();
    eosItem->flags = QV2_WORK_FLAG_EOS;
    std::vector<std::unique_ptr<Qv2Work>> eosItems;
    eosItems.push_back(std::move(eosItem));
    mComponent->queue(std::move(eosItems));

    EXPECT_GT(listener.getFrameCount(), 0);
    fclose(fpIn);
    if(fpOut) {
        fclose(fpOut);
    }
}

class Qv2EncoderY4mTestP : public Qv2EncoderTestP {};

TEST_P(Qv2EncoderY4mTestP, RunY4mEncode) {
    const TestParam& p = GetParam();
    std::string fullInputPath = Y4mInputFolder + p.inputPath;
    QV2_LOGI("Running Y4M encode for: %s", fullInputPath.c_str());

    FILE* fpIn = fopen(fullInputPath.c_str(), "rb");
    ASSERT_NE(fpIn, nullptr) << "Failed to open input file: " << fullInputPath;

    std::string fullOutputPath = Y4mOutputFolder + p.outputPath;
    FILE* fpOut = fopen(fullOutputPath.c_str(), "wb");
    TestListener listener(fpOut);
    mComponent->setListener(&listener);

    y4m_params_t y4mParams;
    ASSERT_GE(y4m_header_parser(fpIn, &y4mParams), 0) << "Failed to parse Y4M header";

    int w = y4mParams.w, h = y4mParams.h, fmt = y4mParams.color_format, bitDepth = y4mParams.bit_depth;
    float fpsValue = (float)y4mParams.fps_num / y4mParams.fps_den;

    std::vector<Qv2Param*> params;
    auto sizeParam = std::make_shared<Qv2VideoSizeInput>();
    sizeParam->mWidth = w; sizeParam->mHeight = h;
    params.push_back(sizeParam.get());

    auto colorParam = std::make_shared<Qv2ColorFormatInput>();
    colorParam->mColorFormat = fmt;
    params.push_back(colorParam.get());

    auto fpsParam = std::make_shared<Qv2FrameRateInput>();
    fpsParam->mFps = fpsValue;
    params.push_back(fpsParam.get());

    auto depthParam = std::make_shared<Qv2BitDepthInput>();
    depthParam->mBitDepth = bitDepth;
    params.push_back(depthParam.get());

    auto bitrateParam = std::make_shared<Qv2BitrateSetting>();
    bitrateParam->mBitrate = 100000000;
    params.push_back(bitrateParam.get());

    auto qpParam = std::make_shared<Qv2QPInput>();
    qpParam->mQP = 25;
    params.push_back(qpParam.get());

    ASSERT_EQ(mComponent->configure(params), QV2_OK);
    mComponent->enableRecon(true);
    ASSERT_EQ(mComponent->start(), QV2_OK);

    int cs = OAPV_CS_SET(toOapvFmt(fmt), bitDepth, 0);
    uint64_t intervalUs = static_cast<uint64_t>(1000000.0f / fpsValue);
    uint64_t currentUs = 0;

    while (true) {
        oapv_imgb_t* imgBuf = imgb_create(w, h, cs);
        if (imgb_read(fpIn, imgBuf, w, h, 1) < 0) {
            imgb_release(imgBuf);
            auto eosItem = std::make_unique<Qv2Work>();
            eosItem->flags = QV2_WORK_FLAG_EOS;
            std::vector<std::unique_ptr<Qv2Work>> eosItems;
            eosItems.push_back(std::move(eosItem));
            mComponent->queue(std::move(eosItems));
            break;
        }

        auto item = std::make_unique<Qv2Work>();
        item->timestamp = currentUs;
        currentUs += intervalUs;

        auto inputBlock = std::make_shared<Qv2Block2D>(w, h, fmt, bitDepth);
        for (int plane = 0; plane < imgBuf->np; ++plane)
            inputBlock->setPlane(plane, (uint8_t*)imgBuf->a[plane], imgBuf->s[plane], imgBuf->h[plane]);
        item->input = Qv2Buffer::CreateGraphicBuffer(inputBlock);

        auto reconBlock = std::make_shared<Qv2Block2D>(w, h, fmt, bitDepth);
        std::vector<std::vector<uint8_t>> reconPlanes(imgBuf->np);
        for (int plane = 0; plane < imgBuf->np; ++plane) {
            reconPlanes[plane].resize(imgBuf->s[plane] * imgBuf->h[plane]);
            reconBlock->setPlane(plane, reconPlanes[plane].data(), imgBuf->s[plane], imgBuf->h[plane]);
        }
        item->recon = Qv2Buffer::CreateGraphicBuffer(reconBlock);

        std::vector<uint8_t> outData(w * h);
        item->output = Qv2Buffer::CreateLinearBuffer(std::make_shared<Qv2Block1D>(outData.data(), 0, outData.size()));

        std::vector<std::unique_ptr<Qv2Work>> items;
        items.push_back(std::move(item));
        mComponent->queue(std::move(items));

        imgb_release(imgBuf);
    }

    EXPECT_GT(listener.getFrameCount(), 0);
    fclose(fpIn);
    if(fpOut) {
        fclose(fpOut);
    }
}

INSTANTIATE_TEST_SUITE_P(
    APV_YUV_Tests,
    Qv2EncoderTestP,
    ::testing::Values(
        TestParam{"/qp_A_yuv422p10le_3840x2160_60fps_3.yuv", "/qp_A_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2_CF_YCBCR422_10LE, 60, 10, 3},
        TestParam{"/qp_B_yuv422p10le_3840x2160_60fps_3.yuv", "/qp_B_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2_CF_YCBCR422_10LE, 60, 10, 3},
        TestParam{"/qp_C_yuv422p10le_3840x2160_60fps_3.yuv", "/qp_C_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2_CF_YCBCR422_10LE, 60, 10, 3},
        TestParam{"/qp_D_yuv422p10le_3840x2160_60fps_3.yuv", "/qp_D_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2_CF_YCBCR422_10LE, 60, 10, 3},
        TestParam{"/qp_E_yuv422p10le_3840x2160_60fps_3.yuv", "/qp_E_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2_CF_YCBCR422_10LE, 60, 10, 3},
        TestParam{"/syn_A_yuv422p10le_1920x1080_60fps_2.yuv", "/syn_A_yuv422p10le_1920x1080_60fps_2.apv", 1920, 1080, QV2_CF_YCBCR422_10LE, 60, 10, 2},
        TestParam{"/syn_B_yuv422p10le_1920x1080_60fps_2.yuv", "/syn_B_yuv422p10le_1920x1080_60fps_2.apv", 1920, 1080, QV2_CF_YCBCR422_10LE, 60, 10, 2},
        TestParam{"/tile_A_yuv422p10le_3840x2160_60fps_3.yuv", "/tile_A_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2_CF_YCBCR422_10LE, 60, 10, 3},
        TestParam{"/tile_B_yuv422p10le_3840x2160_60fps_3.yuv", "/tile_B_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2_CF_YCBCR422_10LE, 60, 10, 3},
        TestParam{"/tile_C_yuv422p10le_7680x4320_30fps_3.yuv", "/tile_C_yuv422p10le_7680x4320_30fps_3.apv", 7680, 4320, QV2_CF_YCBCR422_10LE, 30, 10, 3},
        TestParam{"/tile_D_yuv422p10le_3840x2160_60fps_3.yuv", "/tile_D_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2_CF_YCBCR422_10LE, 60, 10, 3},
        TestParam{"/tile_E_yuv422p10le_3840x2160_60fps_3.yuv", "/tile_E_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2_CF_YCBCR422_10LE, 60, 10, 3}
    )
);

INSTANTIATE_TEST_SUITE_P(
    APV_Y4M_Tests,
    Qv2EncoderY4mTestP,
    ::testing::Values(
        TestParam{"/qp_A_yuv422p10le_3840x2160_60fps_3.y4m", "/qp_A_yuv422p10le_3840x2160_60fps_3.apv"},
        TestParam{"/qp_B_yuv422p10le_3840x2160_60fps_3.y4m", "/qp_B_yuv422p10le_3840x2160_60fps_3.apv"},
        TestParam{"/qp_C_yuv422p10le_3840x2160_60fps_3.y4m", "/qp_C_yuv422p10le_3840x2160_60fps_3.apv"},
        TestParam{"/qp_D_yuv422p10le_3840x2160_60fps_3.y4m", "/qp_D_yuv422p10le_3840x2160_60fps_3.apv"},
        TestParam{"/qp_E_yuv422p10le_3840x2160_60fps_3.y4m", "/qp_E_yuv422p10le_3840x2160_60fps_3.apv"},
        TestParam{"/syn_A_yuv422p10le_1920x1080_60fps_2.y4m", "/syn_A_yuv422p10le_1920x1080_60fps_2.apv"},
        TestParam{"/syn_B_yuv422p10le_1920x1080_60fps_2.y4m", "/syn_B_yuv422p10le_1920x1080_60fps_2.apv"},
        TestParam{"/tile_A_yuv422p10le_3840x2160_60fps_3.y4m", "/tile_A_yuv422p10le_3840x2160_60fps_3.apv"},
        TestParam{"/tile_B_yuv422p10le_3840x2160_60fps_3.y4m", "/tile_B_yuv422p10le_3840x2160_60fps_3.apv"},
        TestParam{"/tile_C_yuv422p10le_7680x4320_30fps_3.y4m", "/tile_C_yuv422p10le_7680x4320_30fps_3.apv"},
        TestParam{"/tile_D_yuv422p10le_3840x2160_60fps_3.y4m", "/tile_D_yuv422p10le_3840x2160_60fps_3.apv"},
        TestParam{"/tile_E_yuv422p10le_3840x2160_60fps_3.y4m", "/tile_E_yuv422p10le_3840x2160_60fps_3.apv"}
    )
);
