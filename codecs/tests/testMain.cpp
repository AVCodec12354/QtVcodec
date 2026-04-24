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
#include <map>

#define LOG_TAG "testMain"

/**
 * @brief Test listener to handle encoded output.
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
                    if (mOutFile) {
                        fwrite(outBlock->data(), 1, outBlock->size(), mOutFile);
                    }
                    mFrameCount++;
                    printf("  [Listener] Encoded frame %d. Size: %zu bytes, TS: %llu\n",
                           mFrameCount, outBlock->size(), (unsigned long long)item->timestamp);
                }
            } else if (item->result != 0) {
                printf("  [Listener] Error processing item: %d\n", item->result);
            }

            if (item->flags & QV2_WORK_FLAG_EOS) {
                printf("  [Listener] EOS reached!\n");
            }

            // Giải phóng tài nguyên liên quan đến work item này
            mResources.erase(item.get());
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

    void keepAlive(Qv2Work* item, std::shared_ptr<void> res) {
        mResources[item].push_back(res);
    }

    int getFrameCount() const { return mFrameCount; }

private:
    FILE* mOutFile;
    int mFrameCount = 0;
    std::map<Qv2Work*, std::vector<std::shared_ptr<void>>> mResources;
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
    Qv2QPInput qp; qp.mQP = 25;

    params.push_back(&size); params.push_back(&fps); params.push_back(&bitrate);
    params.push_back(&depth); params.push_back(&color); params.push_back(&qp);
    component->configure(params);
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
        for (int p = 0; p < imgb->np; ++p) inputBlock->setPlane(p, (uint8_t*)imgb->a[p], imgb->s[p], imgb->h[p]);
        item->input = Qv2Buffer::CreateGraphicBuffer(inputBlock);

        auto outData = std::make_shared<std::vector<uint8_t>>(w * h * 4);
        item->output = Qv2Buffer::CreateLinearBuffer(std::make_shared<Qv2Block1D>(outData->data(), 0, outData->size()));

        // Quản lý lifetime tài nguyên thông qua listener
        listener.keepAlive(item.get(), std::shared_ptr<oapv_imgb_t>(imgb, [](oapv_imgb_t* p){ if(p) p->release(p); }));
        listener.keepAlive(item.get(), outData);

        if (i == maxFrames - 1) item->flags |= QV2_WORK_FLAG_EOS;

        std::vector<std::unique_ptr<Qv2Work>> items;
        items.push_back(std::move(item));
        component->queue(std::move(items));
    }

    printf("  Waiting for completion...\n");
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

        auto outData = std::make_shared<std::vector<uint8_t>>(y4mParams.w * y4mParams.h * 2);
        item->output = Qv2Buffer::CreateLinearBuffer(std::make_shared<Qv2Block1D>(outData->data(), 0, outData->size()));

        // Quản lý lifetime tài nguyên thông qua listener
        listener.keepAlive(item.get(), std::shared_ptr<oapv_imgb_t>(imgb, [](oapv_imgb_t* p){ if(p) p->release(p); }));
        listener.keepAlive(item.get(), outData);

        std::vector<std::unique_ptr<Qv2Work>> items; items.push_back(std::move(item));
        component->queue(std::move(items));
    }

    printf("  Waiting for completion...\n");
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
            // Encode 5 frames of HDR_4k.yuv (3840x2160, YUV422, 10-bit)
            testEncodeRawYUV(inputPath, outputPath, 3840, 2160, OAPV_CF_YCBCR422, 10, 5);
        } else {
            testEncodeY4M(inputPath, outputPath);
        }
    } catch (const std::exception& e) {
        printf("Unhandled exception: %s\n", e.what());
        return 1;
    }
    return 0;
}
