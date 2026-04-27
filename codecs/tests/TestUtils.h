#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <gtest/gtest.h>
#include "Qv2Component.h"
#include "Qv2Buffer.h"
#include "oapv.h"
#include "oapv_app_y4m.h"
#include "oapv_app_util.h"
#include "Qv2Constants.h"
#include "Qv2Log.h"
#include <cstdio>
#include <vector>
#include <memory>
#include <string>
#include <cstring>

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
inline void mapBlockToImgb(std::shared_ptr<Qv2Block2D> block, oapv_imgb_t* imgb) {
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
inline int toOapvFmt(int qv2Format) {
    switch (qv2Format) {
        case QV2_CF_YCBCR420:
            return OAPV_CF_YCBCR420;
        case QV2_CF_YCBCR422:
        case QV2_CF_YCBCR422_10LE:
            return OAPV_CF_YCBCR422;
        case QV2_CF_YCBCR444:
            return OAPV_CF_YCBCR444;
        case QV2_CF_P210:
            return OAPV_CF_PLANAR2;
        default:
            return OAPV_CS_GET_FORMAT(qv2Format);
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
            if (item->flags == QV2_WORK_FLAG_EOS) {
                auto comp = component.lock();
                if (comp) {
                    comp->flush();
                    comp->stop();
                    comp->release();
                }
                if (mOutFile) {
                    fclose(mOutFile);
                    mOutFile = nullptr;
                }
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
        if (mOutFile) {
            fclose(mOutFile);
            mOutFile = nullptr;
        }
    }

    void onStateChanged(std::weak_ptr<Qv2Component> component, Qv2Component::State newState) override {}

    int getFrameCount() const { return mFrameCnt; }

private:
    FILE* mOutFile;
    int mFrameCnt = 0;
};

#endif // TESTUTILS_H
