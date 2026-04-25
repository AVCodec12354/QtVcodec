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
    explicit TestListener(FILE* outFile = nullptr) : mOutFile(outFile) {}

    void onWorkDone(std::weak_ptr<Qv2Component> component,
                    std::vector<std::unique_ptr<Qv2Work>> workItems) override {
        for (auto& item : workItems) {
            if (item->result == 0 && item->output &&
                item->output->type() == Qv2Buffer::LINEAR) {
                if (!item->output->linearBlocks().empty()) {
                    auto outBlock = item->output->linearBlocks()[0];
                    size_t frameSize = outBlock->size();
                    if (mOutFile) {
                        fwrite(outBlock->data(), 1, frameSize, mOutFile);
                    }
                    mFrameCount++;

                    if (item->recon && !item->recon->graphicBlocks().empty() &&
                        item->input && !item->input->graphicBlocks().empty()) {

                        oapv_imgb_t org, rec;
                        mapBlockToImgb(item->input->graphicBlocks()[0], &org);
                        mapBlockToImgb(item->recon->graphicBlocks()[0], &rec);

                        double psnr[4];
                        measure_psnr(&org, &rec, psnr, item->input->graphicBlocks()[0]->bitDepth());

                        // Using QV2_LOGD for per-frame noisy logs
                        QV2_LOGD("    [Frame %d] PSNR Y: %.2f dB", mFrameCount, psnr[0]);
                        EXPECT_GT(psnr[0], 20.0) << "PSNR Y too low on frame " << mFrameCount;
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

    int getFrameCount() const { return mFrameCount; }

private:
    FILE* mOutFile;
    int mFrameCount = 0;
};

/**
 * @brief Parameterized Test Fixture
 */
class Qv2EncoderTestP : public ::testing::TestWithParam<TestParam> {
protected:
    void SetUp() override {
        mComponent = Qv2ComponentFactory::createByType(Qv2ComponentFactory::ENCODER_APV);
        ASSERT_NE(mComponent, nullptr);
    }

    void TearDown() override {
    }

    std::shared_ptr<Qv2Component> mComponent;
};

TEST_P(Qv2EncoderTestP, RunYuvEncode) {
    const TestParam& p = GetParam();

    QV2_LOGI("Running encode for: %s", p.inputPath.c_str());

    FILE* fpIn = fopen(p.inputPath.c_str(), "rb");
    ASSERT_NE(fpIn, nullptr) << "Failed to open input file: " << p.inputPath;

    FILE* fpOut = fopen(p.outputPath.c_str(), "wb");
    TestListener listener(fpOut);
    mComponent->setListener(&listener);

    int w = p.width, h = p.height, colorFormat = p.format, bitDepth = p.depth;
    float fpsValue = static_cast<float>(p.fps);
    bool isY4M = (p.inputPath.find(".y4m") != std::string::npos);

    if (isY4M) {
        y4m_params_t y4mParams;
        ASSERT_GE(y4m_header_parser(fpIn, &y4mParams), 0) << "Failed to parse Y4M header";
        w = y4mParams.w; h = y4mParams.h;
        colorFormat = y4mParams.color_format;
        bitDepth = y4mParams.bit_depth;
        fpsValue = (float)y4mParams.fps_num / y4mParams.fps_den;
    }

    std::vector<Qv2Param*> params;
    Qv2VideoSizeInput sizeParam; sizeParam.mWidth = w; sizeParam.mHeight = h;
    Qv2FrameRateInput fpsParam; fpsParam.mFps = fpsValue;
    Qv2BitrateSetting brParam; brParam.mBitrate = 100000000;
    Qv2BitDepthInput depthParam; depthParam.mBitDepth = bitDepth;
    Qv2ColorFormatInput colorParam; colorParam.mColorFormat = colorFormat;
    Qv2QPInput qpParam; qpParam.mQP = 25;

    params.push_back(&sizeParam); params.push_back(&fpsParam); params.push_back(&brParam);
    params.push_back(&depthParam); params.push_back(&colorParam); params.push_back(&qpParam);

    ASSERT_EQ(mComponent->configure(params), QV2_OK);
    mComponent->enableRecon(true);
    ASSERT_EQ(mComponent->start(), QV2_OK);

    int cs = OAPV_CS_SET(toOapvFmt(colorFormat), bitDepth, 0);
    uint64_t frameDurationUs = static_cast<uint64_t>(1000000.0f / fpsValue);
    uint64_t currentTimestamp = 0;
    int maxFrames = isY4M ? 1000000 : p.numOfFrame;

    for (int i = 0; i < maxFrames; ++i) {
        oapv_imgb_t* imgb = imgb_create(w, h, cs);
        if (imgb_read(fpIn, imgb, w, h, isY4M ? 1 : 0) < 0) {
            imgb->release(imgb);
            break;
        }

        auto item = std::make_unique<Qv2Work>();
        item->timestamp = currentTimestamp;
        currentTimestamp += frameDurationUs;

        auto inputBlock = std::make_shared<Qv2Block2D>(w, h, colorFormat, bitDepth);
        for (int plane = 0; plane < imgb->np; ++plane)
            inputBlock->setPlane(plane, (uint8_t*)imgb->a[plane], imgb->s[plane], imgb->h[plane]);
        item->input = Qv2Buffer::CreateGraphicBuffer(inputBlock);

        auto reconBlock = std::make_shared<Qv2Block2D>(w, h, colorFormat, bitDepth);
        std::vector<std::vector<uint8_t>> reconPlanes(imgb->np);
        for (int plane = 0; plane < imgb->np; ++plane) {
            reconPlanes[plane].resize(imgb->s[plane] * imgb->h[plane]);
            reconBlock->setPlane(plane, reconPlanes[plane].data(), imgb->s[plane], imgb->h[plane]);
        }
        item->recon = Qv2Buffer::CreateGraphicBuffer(reconBlock);

        std::vector<uint8_t> outData(w * h * 4);
        item->output = Qv2Buffer::CreateLinearBuffer(std::make_shared<Qv2Block1D>(outData.data(), 0, outData.size()));

        if (i == maxFrames - 1) item->flags |= QV2_WORK_FLAG_EOS;

        std::vector<std::unique_ptr<Qv2Work>> items;
        items.push_back(std::move(item));
        mComponent->queue(std::move(items));

        imgb->release(imgb);
    }

    EXPECT_GT(listener.getFrameCount(), 0);
    fclose(fpIn);
    if (fpOut) fclose(fpOut);
}

/**
 * @brief List of Raw YUV files to test.
 */
INSTANTIATE_TEST_SUITE_P(
    APV_YUV_Tests,
    Qv2EncoderTestP,
    ::testing::Values(
        TestParam{"YUVTests/input/qp_A_yuv422p10le_3840x2160_60fps_3.yuv", "YUVTests/output/qp_A_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3},
        TestParam{"YUVTests/input/qp_B_yuv422p10le_3840x2160_60fps_3.yuv", "YUVTests/output/qp_B_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3},
        TestParam{"YUVTests/input/qp_C_yuv422p10le_3840x2160_60fps_3.yuv", "YUVTests/output/qp_C_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3},
        TestParam{"YUVTests/input/qp_D_yuv422p10le_3840x2160_60fps_3.yuv", "YUVTests/output/qp_D_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3},
        TestParam{"YUVTests/input/qp_E_yuv422p10le_3840x2160_60fps_3.yuv", "YUVTests/output/qp_E_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3},
        TestParam{"YUVTests/input/syn_A_yuv422p10le_1920x1080_60fps_2.yuv", "YUVTests/output/syn_A_yuv422p10le_1920x1080_60fps_2.apv", 1920, 1080, QV2FormatYUV422Planar, 60, 10, 2},
        TestParam{"YUVTests/input/syn_B_yuv422p10le_1920x1080_60fps_2.yuv", "YUVTests/output/syn_B_yuv422p10le_1920x1080_60fps_2.apv", 1920, 1080, QV2FormatYUV422Planar, 60, 10, 2},
        TestParam{"YUVTests/input/tile_A_yuv422p10le_3840x2160_60fps_3.yuv", "YUVTests/output/tile_A_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3},
        TestParam{"YUVTests/input/tile_B_yuv422p10le_3840x2160_60fps_3.yuv", "YUVTests/output/tile_B_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3},
        TestParam{"YUVTests/input/tile_C_yuv422p10le_7680x4320_30fps_3.yuv", "YUVTests/output/tile_C_yuv422p10le_7680x4320_30fps_3.apv", 7680, 4320, QV2FormatYUV422Planar, 30, 10, 3},
        TestParam{"YUVTests/input/tile_D_yuv422p10le_3840x2160_60fps_3.yuv", "YUVTests/output/tile_D_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3},
        TestParam{"YUVTests/input/tile_E_yuv422p10le_3840x2160_60fps_3.yuv", "YUVTests/output/tile_E_yuv422p10le_3840x2160_60fps_3.apv", 3840, 2160, QV2FormatYUV422Planar, 60, 10, 3}
    )
);

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
