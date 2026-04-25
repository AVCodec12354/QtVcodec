#include "Qv2ComponentFactory.h"
#include "Qv2Buffer.h"
#include "oapv.h"
#include "oapv_app_y4m.h"
#include "oapv_app_util.h"
#include <cstdio>
#include <vector>
#include <memory>
#include <cassert>
#include <iostream>
#include <string>
#include <cstring>

#define LOG_TAG "testMain"

/**
 * @brief Helper to wrap Qv2Block2D into oapv_imgb_t for legacy API usage.
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
 * @brief Test listener to handle encoded output and calculate PSNR.
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

                    std::string psnrLog = "";
                    if (item->recon && !item->recon->graphicBlocks().empty() &&
                        item->input && !item->input->graphicBlocks().empty()) {

                        oapv_imgb_t org, rec;
                        mapBlockToImgb(item->input->graphicBlocks()[0], &org);
                        mapBlockToImgb(item->recon->graphicBlocks()[0], &rec);

                        double psnr[4];
                        measure_psnr(&org, &rec, psnr, item->input->graphicBlocks()[0]->bitDepth());

                        char buf[128];
                        snprintf(buf, sizeof(buf), " PSNR: [Y:%.2f, U:%.2f, V:%.2f]", psnr[0], psnr[1], psnr[2]);
                        psnrLog = buf;
                    }

                    printf("  [Listener] Encoded frame %d. Size: %zu bytes, TS: %llu%s\n",
                           mFrameCount, frameSize, (unsigned long long)item->timestamp, psnrLog.c_str());
                }
            } else if (item->result != 0) {
                printf("  [Listener] Error processing item: %d\n", item->result);
            }

            if (item->flags & QV2_WORK_FLAG_EOS) {
                printf("  [Listener] EOS reached!\n");
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

    int getFrameCount() const { return mFrameCount; }

private:
    FILE* mOutFile;
    int mFrameCount = 0;
};

/**
 * @brief Encode a specific number of frames from a raw YUV file.
 */
void testEncodeRawYUV(const std::string& inputPath, const std::string& outputPath,
                      int w, int h, int colorFormat, int bitDepth, int maxFrames) {
    printf("=== Starting Raw YUV to APV Encoding ===\n");
    printf("Input File:  %s (%dx%d, %d-bit)\n", inputPath.c_str(), w, h, bitDepth);

    FILE* fpIn = fopen(inputPath.c_str(), "rb");
    if (!fpIn) return;

    auto component = Qv2ComponentFactory::createByType(Qv2ComponentFactory::ENCODER_APV);
    FILE* outFile = fopen(outputPath.c_str(), "wb");
    TestListener listener(outFile);
    component->setListener(&listener);

    float fpsValue = 30.0f;
    std::vector<Qv2Param*> params;
    Qv2VideoSizeInput size; size.mWidth = w; size.mHeight = h;
    Qv2FrameRateInput fps; fps.mFps = fpsValue;
    Qv2BitrateSetting bitrate; bitrate.mBitrate = 200000000;
    Qv2BitDepthInput depth; depth.mBitDepth = bitDepth;
    Qv2ColorFormatInput color; color.mColorFormat = colorFormat;
    Qv2QPInput qp; qp.mQP = 51;

    params.push_back(&size); params.push_back(&fps); params.push_back(&bitrate);
    params.push_back(&depth); params.push_back(&color); params.push_back(&qp);
    component->configure(params);
    component->enableRecon(true);
    component->start();

    int cs = OAPV_CS_SET(colorFormat, bitDepth, 0);
    uint64_t frameDurationUs = static_cast<uint64_t>(1000000.0f / fpsValue);
    uint64_t currentTimestamp = 0;

    for (int i = 0; i < maxFrames; ++i) {
        oapv_imgb_t* imgb = imgb_create(w, h, cs);
        if (imgb_read(fpIn, imgb, w, h, 0) < 0) { imgb->release(imgb); break; }

        auto item = std::make_unique<Qv2Work>();
        item->timestamp = currentTimestamp;
        currentTimestamp += frameDurationUs;

        auto inputBlock = std::make_shared<Qv2Block2D>(w, h, colorFormat, bitDepth);
        for (int p = 0; p < imgb->np; ++p)
            inputBlock->setPlane(p, (uint8_t*)imgb->a[p], imgb->s[p], imgb->h[p]);
        item->input = Qv2Buffer::CreateGraphicBuffer(inputBlock);

        // Setup Recon Buffer
        auto reconBlock = std::make_shared<Qv2Block2D>(w, h, colorFormat, bitDepth);
        std::vector<std::vector<uint8_t>> reconPlanes(imgb->np);
        for (int p = 0; p < imgb->np; ++p) {
            reconPlanes[p].resize(imgb->s[p] * imgb->h[p]);
            reconBlock->setPlane(p, reconPlanes[p].data(), imgb->s[p], imgb->h[p]);
        }
        item->recon = Qv2Buffer::CreateGraphicBuffer(reconBlock);

        std::vector<uint8_t> outData(w * h * 4);
        item->output = Qv2Buffer::CreateLinearBuffer(std::make_shared<Qv2Block1D>(outData.data(), 0, outData.size()));

        if (i == maxFrames - 1) item->flags |= QV2_WORK_FLAG_EOS;

        std::vector<std::unique_ptr<Qv2Work>> items;
        items.push_back(std::move(item));

        component->queue(std::move(items));

        imgb->release(imgb);
    }

    component->stop(); component->release();
    fclose(fpIn); fclose(outFile);
}

/**
 * @brief Main encoding test function for Y4M.
 */
void testEncodeY4M(const std::string& inputPath, const std::string& outputPath) {
    printf("=== Starting Y4M to APV Encoding ===\n");
    FILE* fpIn = fopen(inputPath.c_str(), "rb");
    if (!fpIn) return;

    y4m_params_t y4mParams;
    memset(&y4mParams, 0, sizeof(y4mParams));
    if (y4m_header_parser(fpIn, &y4mParams) < 0) { fclose(fpIn); return; }

    auto component = Qv2ComponentFactory::createByType(Qv2ComponentFactory::ENCODER_APV);
    FILE* outFile = fopen(outputPath.c_str(), "wb");
    TestListener listener(outFile);
    component->setListener(&listener);

    float fpsValue = (float)y4mParams.fps_num / y4mParams.fps_den;
    std::vector<Qv2Param*> params;
    Qv2VideoSizeInput size; size.mWidth = y4mParams.w; size.mHeight = y4mParams.h;
    Qv2FrameRateInput fps; fps.mFps = fpsValue;
    Qv2BitrateSetting bitrate; bitrate.mBitrate = 100000000;
    Qv2BitDepthInput depth; depth.mBitDepth = y4mParams.bit_depth;
    Qv2ColorFormatInput color; color.mColorFormat = y4mParams.color_format;
    Qv2QPInput qp; qp.mQP = 25;
    params.push_back(&size); params.push_back(&fps); params.push_back(&bitrate);
    params.push_back(&depth); params.push_back(&color); params.push_back(&qp);

    component->configure(params);
    component->start();

    int cs = OAPV_CS_SET(y4mParams.color_format, y4mParams.bit_depth, 0);
    uint64_t frameDurationUs = static_cast<uint64_t>(1000000.0f / fpsValue);
    uint64_t currentTimestamp = 0;

    while (true) {
        oapv_imgb_t* imgb = imgb_create(y4mParams.w, y4mParams.h, cs);
        if (imgb_read(fpIn, imgb, y4mParams.w, y4mParams.h, 1) < 0) {
            imgb->release(imgb);
            auto item = std::make_unique<Qv2Work>();
            item->flags |= QV2_WORK_FLAG_EOS;
            item->timestamp = currentTimestamp;
            std::vector<std::unique_ptr<Qv2Work>> items;
            items.push_back(std::move(item));
            component->queue(std::move(items));
            break;
        }
        auto item = std::make_unique<Qv2Work>();
        item->timestamp = currentTimestamp;
        currentTimestamp += frameDurationUs;

        auto inputBlock = std::make_shared<Qv2Block2D>(y4mParams.w, y4mParams.h, y4mParams.color_format, y4mParams.bit_depth);
        for (int p = 0; p < imgb->np; ++p) inputBlock->setPlane(p, (uint8_t*)imgb->a[p], imgb->s[p], imgb->h[p]);
        item->input = Qv2Buffer::CreateGraphicBuffer(inputBlock);

        // Setup Recon Buffer
        auto reconBlock = std::make_shared<Qv2Block2D>(y4mParams.w, y4mParams.h, y4mParams.color_format, y4mParams.bit_depth);
        std::vector<std::vector<uint8_t>> reconPlanes(imgb->np);
        for (int p = 0; p < imgb->np; ++p) {
            reconPlanes[p].resize(imgb->s[p] * imgb->h[p]);
            reconBlock->setPlane(p, reconPlanes[p].data(), imgb->s[p], imgb->h[p]);
        }
        item->recon = Qv2Buffer::CreateGraphicBuffer(reconBlock);

        std::vector<uint8_t> outData(y4mParams.w * y4mParams.h * 2);
        item->output = Qv2Buffer::CreateLinearBuffer(std::make_shared<Qv2Block1D>(outData.data(), 0, outData.size()));

        std::vector<std::unique_ptr<Qv2Work>> items; items.push_back(std::move(item));
        component->queue(std::move(items));

        imgb->release(imgb);
    }

    component->stop(); component->release();
    fclose(fpIn); fclose(outFile);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <input.y4m/yuv> [output.apv]\n", argv[0]);
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputPath = (argc > 2) ? argv[2] : "output.apv";

    try {
        if (inputPath.find(".yuv") != std::string::npos) {
            // Encode 5 frames of HDR_4k.yuv (2160x3840, YUV422, 10-bit)
            testEncodeRawYUV(inputPath, outputPath, 2160, 3840, OAPV_CF_YCBCR422, 10, 5);
        } else {
            testEncodeY4M(inputPath, outputPath);
        }
    } catch (const std::exception& e) {
        printf("Unhandled exception: %s\n", e.what());
        return 1;
    }
    return 0;
}
