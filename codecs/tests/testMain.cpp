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
                    if (mFrameCount % 10 == 0 || mFrameCount == 1) {
                        printf("  [Listener] Encoded frame %d. Size: %zu bytes\n",
                               mFrameCount, outBlock->size());
                    }
                }
            } else if (item->result != 0) {
                printf("  [Listener] Error processing item: %d\n", item->result);
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
 * @brief Calculate total frames in Y4M file.
 */
long calculateTotalFrames(FILE* fp, const y4m_params_t& params) {
    double multiplier = 0;
    switch (params.color_format) {
        case OAPV_CF_YCBCR400:  multiplier = 1.0; break;
        case OAPV_CF_YCBCR420:  multiplier = 1.5; break;
        case OAPV_CF_YCBCR422:
        case OAPV_CF_YCBCR422W: multiplier = 2.0; break;
        case OAPV_CF_YCBCR444:  multiplier = 3.0; break;
        case OAPV_CF_YCBCR4444: multiplier = 4.0; break;
        default: multiplier = 1.5; break;
    }
    int bytesPerPixel = (params.bit_depth > 8) ? 2 : 1;
    long frameSize = (long)(params.w * params.h * multiplier * bytesPerPixel);

    long currentPos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, currentPos, SEEK_SET);

    return (fileSize - currentPos) / (frameSize + 6); // +6 for "FRAME\n"
}

/**
 * @brief Main encoding test function using a real Y4M file.
 */
void testEncodeY4M(const std::string& inputPath, const std::string& outputPath) {
    printf("=== Starting Y4M to APV Encoding ===\n");
    printf("Input File:  %s\n", inputPath.c_str());
    printf("Output File: %s\n", outputPath.c_str());

    FILE* fpIn = fopen(inputPath.c_str(), "rb");
    if (!fpIn) {
        printf("Error: Could not open input file %s\n", inputPath.c_str());
        return;
    }

    y4m_params_t y4mParams;
    memset(&y4mParams, 0, sizeof(y4mParams));
    if (y4m_header_parser(fpIn, &y4mParams) < 0) {
        printf("Error: Failed to parse Y4M header\n");
        fclose(fpIn);
        return;
    }

    auto component = Qv2ComponentFactory::createByType(
        Qv2ComponentFactory::ENCODER_APV);
    assert(component != nullptr);

    FILE* outFile = fopen(outputPath.c_str(), "wb");
    if (!outFile) {
        printf("Error: Could not open output file %s\n", outputPath.c_str());
        fclose(fpIn);
        return;
    }

    TestListener listener(outFile);
    component->setListener(&listener);

    // 1. Configure the encoder with parameters from Y4M header
    std::vector<Qv2Param*> params;
    Qv2VideoSizeInput size;
    size.mWidth = y4mParams.w; size.mHeight = y4mParams.h;

    Qv2FrameRateInput fps;
    fps.mFps = static_cast<float>(y4mParams.fps_num) / y4mParams.fps_den;

    Qv2BitrateSetting bitrate;
    bitrate.mBitrate = 100000000; // 100 Mbps

    Qv2BitDepthInput depth;
    depth.mBitDepth = y4mParams.bit_depth;

    Qv2ColorFormatInput color;
    color.mColorFormat = y4mParams.color_format;

    Qv2QPInput qp;
    qp.mQP = 25;

    params.push_back(&size);
    params.push_back(&fps);
    params.push_back(&bitrate);
    params.push_back(&depth);
    params.push_back(&color);
    params.push_back(&qp);

    Qv2Status status = component->configure(params);
    if (status != QV2_OK) {
        printf("Error: Encoder configuration failed with status %d\n", status);
        fclose(fpIn);
        fclose(outFile);
        return;
    }

    // 2. Start the component
    component->start();

    // 3. Process all frames
    long totalFrames = calculateTotalFrames(fpIn, y4mParams);
    printf("Total frames to process: %ld\n", totalFrames);

    int cs = OAPV_CS_SET(y4mParams.color_format, y4mParams.bit_depth, 0);

    for (long i = 0; i < totalFrames; ++i) {
        oapv_imgb_t* imgb = imgb_create(y4mParams.w, y4mParams.h, cs);
        if (imgb_read(fpIn, imgb, y4mParams.w, y4mParams.h, 1) < 0) {
            printf("Warning: End of file or error at frame %ld\n", i);
            imgb->release(imgb);
            break;
        }

        auto item = std::make_unique<Qv2Work>();

        auto inputBlock = std::make_shared<Qv2Block2D>(
            imgb->w[0], imgb->h[0],
            OAPV_CS_GET_FORMAT(imgb->cs),
            OAPV_CS_GET_BIT_DEPTH(imgb->cs));

        for (int p = 0; p < imgb->np; ++p) {
            inputBlock->setPlane(p, reinterpret_cast<uint8_t*>(imgb->a[p]),
                                 imgb->s[p], imgb->h[p]);
        }
        item->input = Qv2Buffer::CreateGraphicBuffer(inputBlock);

        // 4MB per frame is a safe upper bound.
        size_t outSize = y4mParams.w * y4mParams.h * 2;
        std::vector<uint8_t> outData(outSize);
        auto outputBlock = std::make_shared<Qv2Block1D>(
            outData.data(), 0, outData.size());
        item->output = Qv2Buffer::CreateLinearBuffer(outputBlock);

        std::vector<std::unique_ptr<Qv2Work>> items;
        items.push_back(std::move(item));

        component->queue(std::move(items));

        imgb->release(imgb);
    }

    // 4. Clean up
    component->stop();
    component->release();
    fclose(fpIn);
    fclose(outFile);

    printf("=== Encoding Completed ===\n");
    printf("Total frames encoded: %d\n", listener.getFrameCount());
    printf("Output saved to: %s\n\n", outputPath.c_str());
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <input.y4m> [output.apv]\n", argv[0]);
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputPath = (argc > 2) ? argv[2] : "output.apv";

    try {
        testEncodeY4M(inputPath, outputPath);
    } catch (const std::exception& e) {
        printf("Unhandled exception: %s\n", e.what());
        return 1;
    }
    
    return 0;
}
