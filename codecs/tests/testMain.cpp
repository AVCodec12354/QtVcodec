#define LOG_DEBUG 0
#include <gtest/gtest.h>
#include "Qv2ComponentFactory.h"
#include "Qv2Buffer.h"
#include "oapv.h"
#include "oapv_app_y4m.h"
#include "oapv_app_util.h"
#include <cstdio>
#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include <cstring>
#include "Qv2Constants.h"
#include "Qv2Log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "testMain"

static const std::string YuvInputFolder = "YUVTests/input";
static const std::string YuvOutputFolder = "YUVTests/output";

static const std::string Y4mInputFolder = "Y4MTests/input";
static const std::string Y4mOutputFolder = "Y4MTests/output";

/**
 * @brief Structure to hold test parameters for TEST_P
 */
struct TestParam {
    std::string inputPath;
    std::string outputPath;
    int width;
    int height;
    int format;
    int fps;
    int depth;
    int numOfFrame;
};

/**
 * @brief Helper to wrap Qv2Block2D into oapv_imgb_t for PSNR measurement.
 */
void mapBlockToImgb(std::shared_ptr<Qv2Block2D> block, oapv_imgb_t* imgb) {
    std::memset(imgb, 0, sizeof(oapv_imgb_t));
    imgb->cs = OAPV_CS_SET(block->format(), block->bitDepth(), 0);
    imgb->np = block->numPlanes();
    for (uint32_t i = 0; i < (uint32_t)imgb->np; ++i) {
        imgb->w[i] = (i == 0) ? block->width() : (block->width() >> 1);
        imgb->h[i] = block->height();
        imgb->a[i] = block->addr(i);
        imgb->s[i] = block->stride(i);
        imgb->e[i] = block->elevation(i);
    }
}

/**
 * @brief Helper to convert Qv2 format to OAPV IDC.
 */
int toOapvFmt(int qv2Format) {
    switch (qv2Format) {
        case QV2FormatYUV420Planar:
        case QV2FormatYUV420Flexible:
            return OAPV_CF_YCBCR420;
        case QV2FormatYUV422Planar:
        case QV2FormatYUV422Flexible:
            return OAPV_CF_YCBCR422;
        case QV2FormatYUV444Flexible:
            return OAPV_CF_YCBCR444;
        case QV2FormatYUVP010:
            return OAPV_CF_PLANAR2;
        default:
            return OAPV_CF_YCBCR422;
    }
}

/**
 * @brief Test listener to handle encoded output and verify results.
 */
class TestListener : public Qv2Component::Listener {
public:
    explicit TestListener(FILE* outFile = nullptr) : mOutFile(outFile) {

    }

    void onWorkDone(std::weak_ptr<Qv2Component> component,
                    std::vector<std::unique_ptr<Qv2Work>> workItems) override {
        for (auto& item : workItems) {
            if (item->flags == QV2_WORK_FLAG_EOS) {
                auto comp = component.lock();
                comp->flush();
                comp->stop();
                comp->release();
                if (mOutFile)
                    fclose(mOutFile);
                break;
            }
            if (item->result == 0 && item->output &&
                item->output->type() == Qv2Buffer::LINEAR) {
                if (!item->output->linearBlocks().empty()) {
                    auto outBlock = item->output->linearBlocks()[0];
                    size_t frameSize = outBlock->size();
                    if (mOutFile) {
                        fwrite(outBlock->data(), 1, frameSize, mOutFile);
                    }
                    mFrameCnt++;

                    if (item->recon && !item->recon->graphicBlocks().empty() &&
                        item->input && !item->input->graphicBlocks().empty()) {

                        oapv_imgb_t org, rec;
                        mapBlockToImgb(item->input->graphicBlocks()[0], &org);
                        mapBlockToImgb(item->recon->graphicBlocks()[0], &rec);

                        double psnr[4];
                        measure_psnr(&org, &rec, psnr, item->input->graphicBlocks()[0]->bitDepth());

                        QV2_LOGD("    [Frame %d] [TS: %llu] PSNR Y: %.2f dB PSNR U: %.2f dB PSNR V: %.2f dB",
                                 mFrameCnt, (unsigned long long)item->timestamp, psnr[0], psnr[1], psnr[2]);
                    }
                }
            } else if (item->result != 0) {
                ADD_FAILURE() << "Work item failed with result: " << item->result;
            }
        }
    }

    void onError(std::weak_ptr<Qv2Component> component, Qv2Status error) override {
        QV2_LOGE("Component error received: %d", static_cast<int>(error));
        ADD_FAILURE() << "Component error received: " << static_cast<int>(error);
    }

    void onStateChanged(std::weak_ptr<Qv2Component> component, Qv2Component::State newState) override {}

    int getFrameCount() const { return mFrameCnt; }

private:
    FILE* mOutFile;
    int mFrameCnt = 0;
};

/**
 * @brief Parameterized Test Fixture
 */
class Qv2EncoderTestP : public ::testing::TestWithParam<TestParam> {
protected:
    void SetUp() override {
        QV2_LOGI("This is %s", __func__);
        mComponent = Qv2ComponentFactory::createByType(Qv2ComponentFactory::ENCODER_APV);
        ASSERT_NE(mComponent, nullptr);
    }

    void TearDown() override {
        QV2_LOGI("This is %s", __func__);
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

    QV2_LOGI("Qv2 Setting params");
    std::vector<Qv2Param*> params;

    std::shared_ptr<Qv2VideoSizeInput> sizeParam = std::make_shared<Qv2VideoSizeInput>();
    sizeParam->mWidth = w;
    sizeParam->mHeight = h;
    params.push_back(sizeParam.get());

    std::shared_ptr<Qv2ColorFormatInput> colorParam = std::make_shared<Qv2ColorFormatInput>();
    colorParam->mColorFormat = fmt;
    params.push_back(colorParam.get());

    std::shared_ptr<Qv2FrameRateInput> fpsParam = std::make_shared<Qv2FrameRateInput>();
    fpsParam->mFps = fpsValue;
    params.push_back(fpsParam.get());

    std::shared_ptr<Qv2BitDepthInput> depthParam = std::make_shared<Qv2BitDepthInput>();
    depthParam->mBitDepth = bitDepth;
    params.push_back(depthParam.get());

    std::shared_ptr<Qv2BitrateSetting> bitrateParam = std::make_shared<Qv2BitrateSetting>();
    bitrateParam->mBitrate = 100000000;
    params.push_back(bitrateParam.get());

    std::shared_ptr<Qv2QPInput> qpParam = std::make_shared<Qv2QPInput>();
    qpParam->mQP = 25;
    params.push_back(qpParam.get());

    QV2_LOGI("Qv2 configure params");
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

    auto item = std::make_unique<Qv2Work>();
    item->flags = QV2_WORK_FLAG_EOS;
    item->timestamp = 0;
    std::vector<std::unique_ptr<Qv2Work>> items;
    items.push_back(std::move(item));
    mComponent->queue(std::move(items));

    EXPECT_GT(listener.getFrameCount(), 0);
    fclose(fpIn);
    if (fpOut)
        fclose(fpOut);
}

/**
 * @brief Parameterized Test Fixture for Y4M
 */
class Qv2EncoderY4mTestP : public Qv2EncoderTestP {

};

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
    std::shared_ptr<Qv2VideoSizeInput> sizeParam = std::make_shared<Qv2VideoSizeInput>();
    sizeParam->mWidth = w; sizeParam->mHeight = h;
    params.push_back(sizeParam.get());

    std::shared_ptr<Qv2ColorFormatInput> colorParam = std::make_shared<Qv2ColorFormatInput>();
    colorParam->mColorFormat = fmt;
    params.push_back(colorParam.get());

    std::shared_ptr<Qv2FrameRateInput> fpsParam = std::make_shared<Qv2FrameRateInput>();
    fpsParam->mFps = fpsValue;
    params.push_back(fpsParam.get());

    std::shared_ptr<Qv2BitDepthInput> depthParam = std::make_shared<Qv2BitDepthInput>();
    depthParam->mBitDepth = bitDepth;
    params.push_back(depthParam.get());

    std::shared_ptr<Qv2BitrateSetting> bitrateParam = std::make_shared<Qv2BitrateSetting>();
    bitrateParam->mBitrate = 100000000;
    params.push_back(bitrateParam.get());

    std::shared_ptr<Qv2QPInput> qpParam = std::make_shared<Qv2QPInput>();
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

            auto item = std::make_unique<Qv2Work>();
            item->flags = QV2_WORK_FLAG_EOS;
            item->timestamp = 0;
            std::vector<std::unique_ptr<Qv2Work>> items;
            items.push_back(std::move(item));
            mComponent->queue(std::move(items));
            break;
        }

        auto item = std::make_unique<Qv2Work>();
        item->timestamp = currentUs;
        currentUs += intervalUs;

        auto inputBlock = std::make_shared<Qv2Block2D>(w, h, fmt, bitDepth);
        for (int p = 0; p < imgBuf->np; ++p)
            inputBlock->setPlane(p, (uint8_t*)imgBuf->a[p], imgBuf->s[p], imgBuf->h[p]);
        item->input = Qv2Buffer::CreateGraphicBuffer(inputBlock);

        auto reconBlock = std::make_shared<Qv2Block2D>(w, h, fmt, bitDepth);
        std::vector<std::vector<uint8_t>> reconPlanes(imgBuf->np);
        for (int p = 0; p < imgBuf->np; ++p) {
            reconPlanes[p].resize(imgBuf->s[p] * imgBuf->h[p]);
            reconBlock->setPlane(p, reconPlanes[p].data(), imgBuf->s[p], imgBuf->h[p]);
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
    if (fpOut)
        fclose(fpOut);
}

/**
 * @brief List of Raw YUV files to test.
 */
INSTANTIATE_TEST_SUITE_P(
    APV_YUV_Tests,
    Qv2EncoderTestP,
    ::testing::Values(
        TestParam{"/qp_A_yuv422p10le_3840x2160_60fps_3.yuv", "/qp_A_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3},
        TestParam{"/qp_B_yuv422p10le_3840x2160_60fps_3.yuv", "/qp_B_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3},
        TestParam{"/qp_C_yuv422p10le_3840x2160_60fps_3.yuv", "/qp_C_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3},
        TestParam{"/qp_D_yuv422p10le_3840x2160_60fps_3.yuv", "/qp_D_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3},
        TestParam{"/qp_E_yuv422p10le_3840x2160_60fps_3.yuv", "/qp_E_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3},
        TestParam{"/syn_A_yuv422p10le_1920x1080_60fps_2.yuv", "/syn_A_yuv422p10le_1920x1080_60fps_2.apv", 1920, 1080, QV2FormatYUV422Planar, 60, 10, 2},
        TestParam{"/syn_B_yuv422p10le_1920x1080_60fps_2.yuv", "/syn_B_yuv422p10le_1920x1080_60fps_2.apv", 1920, 1080, QV2FormatYUV422Planar, 60, 10, 2},
        TestParam{"/tile_A_yuv422p10le_3840x2160_60fps_3.yuv", "/tile_A_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3},
        TestParam{"/tile_B_yuv422p10le_3840x2160_60fps_3.yuv", "/tile_B_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3},
        TestParam{"/tile_C_yuv422p10le_7680x4320_30fps_3.yuv", "/tile_C_yuv422p10le_7680x4320_30fps_3.apv", 7680, 4320, QV2FormatYUV422Planar, 30, 10, 3},
        TestParam{"/tile_D_yuv422p10le_3840x2160_60fps_3.yuv", "/tile_D_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3},
        TestParam{"/tile_E_yuv422p10le_3840x2160_60fps_3.yuv", "/tile_E_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3}
    )
);

/**
 * @brief List of Raw Y4M files to test.
 */
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

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
