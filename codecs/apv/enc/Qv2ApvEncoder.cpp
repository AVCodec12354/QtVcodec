#define LOG_DEBUG 0
#include "Qv2ApvEncoder.h"
#define LOG_TAG "Qv2ApvEncoder"

#include "Qv2Log.h"
#include "../oapv_app_util.h"
#include <cstring>
#include <cstdio>
#include <limits>
#include "Qv2Constants.h"

#define MAX_BS_BUF   (128 * 1024 * 1024)
#define MAX_NUM_FRMS (1)
#define FRM_IDX      (0)
constexpr char COMPONENT_NAME[] = "qv2.apv.encoder";

Qv2ApvEncoder::Qv2ApvEncoder()
        : mEncoderId(nullptr), mMetaDataId(nullptr), mBitstreamBuf(nullptr) {

    setState(UNINITIALIZED);

    QV2_LOGV("Constructor called.");
    mName = COMPONENT_NAME;
    mCodecDesc = std::make_unique<oapve_cdesc_t>();

    std::memset(mCodecDesc.get(), 0, sizeof(oapve_cdesc_t));

    mCodecDesc->max_bs_buf_size = MAX_BS_BUF;
    mCodecDesc->max_num_frms = MAX_NUM_FRMS;
    mCodecDesc->threads = OAPV_CDESC_THREADS_AUTO;

    int ret = OAPV_OK;
    for (int32_t i = 0; i < MAX_NUM_FRMS; i++) {
        oapve_param_t *param = &mCodecDesc->param[i];
        ret = oapve_param_default(param);
        if (OAPV_FAILED(ret)) {
            QV2_LOGE("cannot set default parameter");
        }
    }
    QV2_LOGV("APV Encoder Configuration default");
    showEncoderParams(mCodecDesc.get());
    setState(INITIALIZED);
}

Qv2ApvEncoder::~Qv2ApvEncoder() {
    QV2_LOGV("Destructor called.");
}

std::string Qv2ApvEncoder::getVersion() const {
    unsigned int ver_num;
    const char *ver_str = oapv_version(&ver_num);
    return std::string("OpenAPV ") + ver_str;
}

Qv2Status Qv2ApvEncoder::configure(const std::vector<Qv2Param *> &params) {
    QV2_LOGV("configure() entry. Params size: %zu", params.size());

    if (mState != INITIALIZED && mState != STOPPED) {
        QV2_LOGW("configure() failed: Invalid state %s",
                 Qv2Component::stateToString(mState).c_str());
        return QV2_ERR_INTERNAL;
    }

    oapve_param_t *p = &mCodecDesc->param[FRM_IDX];

    for (auto param: params) {
        if (!param) continue;

        switch (param->mId) {
            case Qv2VideoSizeInput::ID: {
                auto v = static_cast<Qv2VideoSizeInput *>(param);
                p->w = v->mWidth;
                p->h = v->mHeight;
                p->tile_w = ALIGN_VAL(p->w, OAPV_MB_W);
                p->tile_h = ALIGN_VAL(p->h, OAPV_MB_H);
                QV2_LOGV("Set VideoSize: %dx%d, Tile: %dx%d",
                         p->w, p->h, p->tile_w, p->tile_h);
                break;
            }
            case Qv2BitrateSetting::ID: {
                auto v = static_cast<Qv2BitrateSetting *>(param);
                p->bitrate = v->mBitrate / 1000; // bps to kbps
                p->rc_type = OAPV_RC_ABR;
                QV2_LOGV("Set Bitrate: %d kbps", p->bitrate);
                break;
            }
            case Qv2FrameRateInput::ID: {
                auto v = static_cast<Qv2FrameRateInput *>(param);
                p->fps_num = static_cast<int>(v->mFps);
                p->fps_den = 1;
                QV2_LOGV("Set FPS: %d", p->fps_num);
                break;
            }
            case Qv2BitDepthInput::ID: {
                auto v = static_cast<Qv2BitDepthInput *>(param);
                if (v->mBitDepth != 8 && v->mBitDepth != 10 && v->mBitDepth != 12) {
                    QV2_LOGE("Invalid input bit depth: %d, must be 8, 10, or 12", v->mBitDepth);
                    return QV2_ERR_INVALID_ARG;
                }
                mInputDepth = v->mBitDepth;
                QV2_LOGV("Set BitDepth: %d", mInputDepth);
                break;
            }
            case Qv2ColorFormatInput::ID: {
                auto v = static_cast<Qv2ColorFormatInput *>(param);
                mColorFmt = toOapvFmt(v->mColorFormat);
                QV2_LOGV("ColorFormat translated from %d to OAPV IDC: %d", v->mColorFormat, mColorFmt);
                break;
            }
            case Qv2ProfileOutput::ID: {
                auto v = static_cast<Qv2ProfileOutput *>(param);
                p->profile_idc = v->mProfile;
                QV2_LOGV("Set Profile: %d", p->profile_idc);
                break;
            }
            case Qv2LevelOutput::ID: {
                auto v = static_cast<Qv2LevelOutput *>(param);
                p->level_idc = v->mLevel;
                QV2_LOGV("Set Level: %d", p->level_idc);
                break;
            }
            case Qv2BandOutput::ID: {
                auto v = static_cast<Qv2BandOutput *>(param);
                p->band_idc = v->mBand;
                QV2_LOGV("Set Band: %d", p->band_idc);
                break;
            }
            case Qv2QPInput::ID: {
                auto v = static_cast<Qv2QPInput *>(param);
                int maxQP = 63 + (mInputDepth - 10) * 6;
                if (v->mQP < 0 || v->mQP > maxQP) {
                    QV2_LOGE("Invalid QP: %d, must be 0 to %d for bitdepth %d", v->mQP, maxQP, mInputDepth);
                    return QV2_ERR_INVALID_ARG;
                }
                p->qp = static_cast<unsigned char>(v->mQP);
                QV2_LOGV("Set QP: %d", (int) p->qp);
                break;
            }
            case Qv2ThreadsSetting::ID: {
                auto v = static_cast<Qv2ThreadsSetting *>(param);
                if (v->mThreads == "auto") {
                    mCodecDesc->threads = OAPV_CDESC_THREADS_AUTO;
                } else {
                    int threads = std::stoi(v->mThreads);
                    if (threads < 1) {
                        QV2_LOGE("Invalid threads: %d, must be > 0 or 'auto'", threads);
                        return QV2_ERR_INVALID_ARG;
                    }
                    mCodecDesc->threads = threads;
                }
                QV2_LOGV("Set Threads: %s", v->mThreads.c_str());
                break;
            }
            case Qv2PresetSetting::ID: {
                auto v = static_cast<Qv2PresetSetting *>(param);
                if (v->mPreset == "fastest") {
                    p->preset = 0;
                } else if (v->mPreset == "fast") {
                    p->preset = 1;
                } else if (v->mPreset == "medium") {
                    p->preset = 2;
                } else if (v->mPreset == "slow") {
                    p->preset = 3;
                } else if (v->mPreset == "placebo") {
                    p->preset = 4;
                } else {
                    QV2_LOGE("Invalid preset: %s", v->mPreset.c_str());
                    return QV2_ERR_INVALID_ARG;
                }
                QV2_LOGV("Set Preset: %s (%d)", v->mPreset.c_str(), p->preset);
                break;
            }
            case Qv2FamilySetting::ID: {
                auto v = static_cast<Qv2FamilySetting *>(param);
                // Family is string, but may need mapping or validation
                QV2_LOGV("Set Family: %s", v->mFamily.c_str());
                // Note: Family and bitrate cannot be set together, but validation later
                break;
            }
            case Qv2MaxAUSetting::ID: {
                auto v = static_cast<Qv2MaxAUSetting *>(param);
                if (v->mMaxAU < 0) {
                    QV2_LOGE("Invalid max-au: %d, must be >= 0", v->mMaxAU);
                    return QV2_ERR_INVALID_ARG;
                }
                // Max AU is global, not in oapve_param_t
                QV2_LOGV("Set Max AU: %d", v->mMaxAU);
                break;
            }
            case Qv2SeekSetting::ID: {
                auto v = static_cast<Qv2SeekSetting *>(param);
                if (v->mSeek < 0) {
                    QV2_LOGE("Invalid seek: %d, must be >= 0", v->mSeek);
                    return QV2_ERR_INVALID_ARG;
                }
                // Seek is global
                QV2_LOGV("Set Seek: %d", v->mSeek);
                break;
            }
            case Qv2QPOffsetC1Setting::ID: {
                auto v = static_cast<Qv2QPOffsetC1Setting *>(param);
                p->qp_offset_c1 = std::stoi(v->mQPOffsetC1);
                QV2_LOGV("Set QP Offset C1: %d", p->qp_offset_c1);
                break;
            }
            case Qv2QPOffsetC2Setting::ID: {
                auto v = static_cast<Qv2QPOffsetC2Setting *>(param);
                p->qp_offset_c2 = std::stoi(v->mQPOffsetC2);
                QV2_LOGV("Set QP Offset C2: %d", p->qp_offset_c2);
                break;
            }
            case Qv2QPOffsetC3Setting::ID: {
                auto v = static_cast<Qv2QPOffsetC3Setting *>(param);
                p->qp_offset_c3 = std::stoi(v->mQPOffsetC3);
                QV2_LOGV("Set QP Offset C3: %d", p->qp_offset_c3);
                break;
            }
            case Qv2TileWidthSetting::ID: {
                auto v = static_cast<Qv2TileWidthSetting *>(param);
                p->tile_w = std::stoi(v->mTileWidth);
                QV2_LOGV("Set Tile Width: %d", p->tile_w);
                break;
            }
            case Qv2TileHeightSetting::ID: {
                auto v = static_cast<Qv2TileHeightSetting *>(param);
                p->tile_h = std::stoi(v->mTileHeight);
                QV2_LOGV("Set Tile Height: %d", p->tile_h);
                break;
            }
            case Qv2QMatrixC0Setting::ID: {
                auto v = static_cast<Qv2QMatrixC0Setting *>(param);
                // Parse q matrix string
                p->use_q_matrix = 1;
                // Assume parsing logic here
                QV2_LOGV("Set Q Matrix C0: %s", v->mQMatrixC0.c_str());
                break;
            }
            case Qv2QMatrixC1Setting::ID: {
                auto v = static_cast<Qv2QMatrixC1Setting *>(param);
                p->use_q_matrix = 1;
                QV2_LOGV("Set Q Matrix C1: %s", v->mQMatrixC1.c_str());
                break;
            }
            case Qv2QMatrixC2Setting::ID: {
                auto v = static_cast<Qv2QMatrixC2Setting *>(param);
                p->use_q_matrix = 1;
                QV2_LOGV("Set Q Matrix C2: %s", v->mQMatrixC2.c_str());
                break;
            }
            case Qv2QMatrixC3Setting::ID: {
                auto v = static_cast<Qv2QMatrixC3Setting *>(param);
                p->use_q_matrix = 1;
                QV2_LOGV("Set Q Matrix C3: %s", v->mQMatrixC3.c_str());
                break;
            }
            case Qv2ColorPrimariesSetting::ID: {
                auto v = static_cast<Qv2ColorPrimariesSetting *>(param);
                if (v->mColorPrimaries < 1 || v->mColorPrimaries > 12) {
                    QV2_LOGE("Invalid color primaries: %d", v->mColorPrimaries);
                    return QV2_ERR_INVALID_ARG;
                }
                p->color_primaries = v->mColorPrimaries;
                p->color_description_present_flag = 1;
                QV2_LOGV("Set Color Primaries: %d", p->color_primaries);
                break;
            }
            case Qv2ColorTransferSetting::ID: {
                auto v = static_cast<Qv2ColorTransferSetting *>(param);
                if (v->mColorTransfer < 1 || v->mColorTransfer > 18) {
                    QV2_LOGE("Invalid color transfer: %d", v->mColorTransfer);
                    return QV2_ERR_INVALID_ARG;
                }
                p->transfer_characteristics = v->mColorTransfer;
                p->color_description_present_flag = 1;
                QV2_LOGV("Set Color Transfer: %d", p->transfer_characteristics);
                break;
            }
            case Qv2ColorMatrixSetting::ID: {
                auto v = static_cast<Qv2ColorMatrixSetting *>(param);
                if (v->mColorMatrix < 0 || v->mColorMatrix > 14) {
                    QV2_LOGE("Invalid color matrix: %d", v->mColorMatrix);
                    return QV2_ERR_INVALID_ARG;
                }
                p->matrix_coefficients = v->mColorMatrix;
                p->color_description_present_flag = 1;
                QV2_LOGV("Set Color Matrix: %d", p->matrix_coefficients);
                break;
            }
            case Qv2ColorRangeSetting::ID: {
                auto v = static_cast<Qv2ColorRangeSetting *>(param);
                if (v->mColorRange != 0 && v->mColorRange != 1) {
                    QV2_LOGE("Invalid color range: %d, must be 0 or 1", v->mColorRange);
                    return QV2_ERR_INVALID_ARG;
                }
                p->full_range_flag = v->mColorRange;
                p->color_description_present_flag = 1;
                QV2_LOGV("Set Color Range: %d", p->full_range_flag);
                break;
            }
            case Qv2HashSetting::ID: {
                auto v = static_cast<Qv2HashSetting *>(param);
                mUseHash = v->mHash;
                QV2_LOGV("Set Hash: %d", mUseHash);
                break;
            }
             case Qv2MasterDisplaySetting::ID: {
                 auto v = static_cast<Qv2MasterDisplaySetting *>(param);
                 // Validate master display format: G(x,y)B(x,y)R(x,y)WP(x,y)L(max,min)
                 if (!v->mMasterDisplay.empty()) {
                     oapvm_payload_mdcv_t mdcv;
                     std::memset(&mdcv, 0, sizeof(oapvm_payload_mdcv_t));
                     if (parseMasterDisplay(v->mMasterDisplay.c_str(), &mdcv) != 0) {
                         QV2_LOGE("Invalid master display format: %s", v->mMasterDisplay.c_str());
                         return QV2_ERR_INVALID_ARG;
                     }
                 }
                 mMasterDisplay = v->mMasterDisplay;
                 QV2_LOGV("Set Master Display: %s", mMasterDisplay.c_str());
                 break;
             }
             case Qv2MaxCLLSetting::ID: {
                 auto v = static_cast<Qv2MaxCLLSetting *>(param);
                 // Validate max CLL format: max_cll,max_fall
                 if (!v->mMaxCLL.empty()) {
                     oapvm_payload_cll_t cll;
                     std::memset(&cll, 0, sizeof(oapvm_payload_cll_t));
                     if (parseMaxCLL(v->mMaxCLL.c_str(), &cll) != 0) {
                         QV2_LOGE("Invalid max CLL format: %s", v->mMaxCLL.c_str());
                         return QV2_ERR_INVALID_ARG;
                     }
                 }
                 mMaxCLL = v->mMaxCLL;
                 QV2_LOGV("Set Max CLL: %s", mMaxCLL.c_str());
                 break;
             }
            default:
                break;
        }
    }
    showEncoderParams(mCodecDesc.get());
    setState(CONFIGURED);
    return QV2_OK;
}

Qv2Status Qv2ApvEncoder::query(std::vector<Qv2Param *> &params) const {
    return QV2_OK;
}

int Qv2ApvEncoder::parseMasterDisplay(const char* data_string, oapvm_payload_mdcv_t *mdcv) const {
    double gx, gy, bx, by, rx, ry, wpx, wpy, max_l, min_l;
    int assigned_fields = std::sscanf(data_string,
        "G(%lf,%lf)B(%lf,%lf)R(%lf,%lf)WP(%lf,%lf)L(%lf,%lf)",
        &gx, &gy, &bx, &by, &rx, &ry, &wpx, &wpy, &max_l, &min_l
    );

    const int expected_fields = 10;
    if (assigned_fields != expected_fields) {
        QV2_LOGE("Parsing error: master display color volume information (parsed %d fields, expected %d)",
                 assigned_fields, expected_fields);
        return -1;
    }

    mdcv->primary_chromaticity_x[1] = (int)(gx * 50000.0 + 0.5);
    mdcv->primary_chromaticity_y[1] = (int)(gy * 50000.0 + 0.5);
    mdcv->primary_chromaticity_x[2] = (int)(bx * 50000.0 + 0.5);
    mdcv->primary_chromaticity_y[2] = (int)(by * 50000.0 + 0.5);
    mdcv->primary_chromaticity_x[0] = (int)(rx * 50000.0 + 0.5);
    mdcv->primary_chromaticity_y[0] = (int)(ry * 50000.0 + 0.5);
    mdcv->white_point_chromaticity_x = (int)(wpx * 50000.0 + 0.5);
    mdcv->white_point_chromaticity_y = (int)(wpy * 50000.0 + 0.5);
    mdcv->max_mastering_luminance = (unsigned long)(max_l * 10000.0 + 0.5);
    mdcv->min_mastering_luminance = (unsigned long)(min_l * 10000.0 + 0.5);

    // Validate chromaticity coordinates are within 0-50000 range
    // (these represent x * 50000 and y * 50000 for 0-1 range)
    if (mdcv->primary_chromaticity_x[0] > 50000 || mdcv->primary_chromaticity_y[0] > 50000 ||
        mdcv->primary_chromaticity_x[1] > 50000 || mdcv->primary_chromaticity_y[1] > 50000 ||
        mdcv->primary_chromaticity_x[2] > 50000 || mdcv->primary_chromaticity_y[2] > 50000 ||
        mdcv->white_point_chromaticity_x > 50000 || mdcv->white_point_chromaticity_y > 50000) {
        QV2_LOGE("Master display chromaticity values out of range (max 50000)");
        return -1;
    }

    // Validate luminance: max must be >= min
    if (mdcv->max_mastering_luminance < mdcv->min_mastering_luminance) {
        QV2_LOGE("Master display max luminance (%lu) must be >= min luminance (%lu)",
                 mdcv->max_mastering_luminance, mdcv->min_mastering_luminance);
        return -1;
    }

    return 0;
}

int Qv2ApvEncoder::parseMaxCLL(const char* data_string, oapvm_payload_cll_t *cll) const {
    double max_cll, max_fall;
    int assigned_fields = std::sscanf(data_string,
        "%lf,%lf",
        &max_cll, &max_fall
    );

    const int expected_fields = 2;
    if (assigned_fields != expected_fields) {
        QV2_LOGE("Parsing error: content light level information (parsed %d fields, expected %d)",
                 assigned_fields, expected_fields);
        return -1;
    }

    cll->max_cll = (int)(max_cll + 0.5);
    cll->max_fall = (int)(max_fall + 0.5);

    // Validate max_cll >= max_fall (max_cll should be at least as large)
    if (cll->max_cll < cll->max_fall) {
        QV2_LOGE("Invalid max CLL: max_cll (%d) should be >= max_fall (%d)",
                 cll->max_cll, cll->max_fall);
        return -1;
    }

    return 0;
}

Qv2Status Qv2ApvEncoder::updateMetadata() const {
    if (!mMetaDataId) {
        return QV2_ERR_NOT_INITIALIZED;
    }

    int ret = 0, size;
    oapvm_payload_mdcv_t mdcv;
    oapvm_payload_cll_t cll;
    int is_mdcv, is_cll;
    unsigned char payload[64];

    is_mdcv = (mMasterDisplay.length() > 0) ? 1 : 0;
    is_cll = (mMaxCLL.length() > 0) ? 1 : 0;

    if (!is_mdcv && !is_cll) {
        // no need to add metadata payload
        return QV2_OK;
    }

    if (is_mdcv) {
        std::memset(&mdcv, 0, sizeof(oapvm_payload_mdcv_t));
        if (parseMasterDisplay(mMasterDisplay.c_str(), &mdcv)) {
            QV2_LOGE("cannot parse master display information");
            return QV2_ERR_INVALID_ARG;
        }
        if (OAPV_FAILED(oapvm_write_mdcv(&mdcv, payload, &size))) {
            QV2_LOGE("cannot get master display information bitstream");
            return QV2_ERR_INTERNAL;
        }
        if (OAPV_FAILED(oapvm_set(mMetaDataId, 1, OAPV_METADATA_MDCV, payload, size))) {
            QV2_LOGE("cannot set master display information to handler");
            return QV2_ERR_INTERNAL;
        }
        QV2_LOGV("Master Display metadata set successfully. Payload size: %d", size);
    }

    if (is_cll) {
        std::memset(&cll, 0, sizeof(oapvm_payload_cll_t));
        if (parseMaxCLL(mMaxCLL.c_str(), &cll)) {
            QV2_LOGE("cannot parse content light level information");
            return QV2_ERR_INVALID_ARG;
        }
        if (OAPV_FAILED(oapvm_write_cll(&cll, payload, &size))) {
            QV2_LOGE("cannot get content light level information bitstream");
            return QV2_ERR_INTERNAL;
        }
        if (OAPV_FAILED(oapvm_set(mMetaDataId, 1, OAPV_METADATA_CLL, payload, size))) {
            QV2_LOGE("cannot set content light level information to handler");
            return QV2_ERR_INTERNAL;
        }
        QV2_LOGV("Max CLL metadata set successfully. Payload size: %d", size);
    }

    return QV2_OK;
}

Qv2Status Qv2ApvEncoder::queue(std::vector <std::unique_ptr<Qv2Work>> items) {
    QV2_LOGV("queue() entry. Items size: %zu", items.size());

    if (mState != RUNNING) {
        QV2_LOGW("queue() failed: Invalid state %s",
                 Qv2Component::stateToString(mState).c_str());
        return QV2_ERR_INTERNAL;
    }

    if (!mEncoderId) {
        QV2_LOGE("queue() failed: encoder is not created");
        return QV2_ERR_NOT_INITIALIZED;
    }

    if (items.empty()) {
        return QV2_OK;
    }

    int codecDepth = getCodecBitDepth(mCodecDesc->param[FRM_IDX].profile_idc);
    QV2_LOGV("Codec depth: %d, Input depth: %d, Color Format: %d",
             codecDepth, mInputDepth, mColorFmt);

    Qv2Status status = QV2_OK;
    for (auto &item: items) {
        if (!item || !item->input) continue;

        if (item->input->graphicBlocks().empty()) {
            QV2_LOGE("Input buffer has no graphic blocks");
            item->result = QV2_ERR_INVALID_ARG;
            continue;
        }

        auto block = item->input->graphicBlocks()[0];

        oapv_frms_t inputFrames;
        oapv_imgb_t srcImgb;

        mapBlockToImgb(block, &srcImgb, mInputDepth);

        std::memset(&inputFrames, 0, sizeof(oapv_frms_t));
        inputFrames.num_frms = 1;
        inputFrames.frm[FRM_IDX].group_id = 1;
        inputFrames.frm[FRM_IDX].pbu_type = OAPV_PBU_TYPE_PRIMARY_FRAME;

        if (codecDepth != mInputDepth) {
            QV2_LOGE("can not support if codecDepth != mInputDepth");
            return QV2_ERR_UNSUPPORTED;
        } else {
            inputFrames.frm[FRM_IDX].imgb = &srcImgb;
        }

        oapv_frms_t reconFrames;
        oapv_imgb_t reconImgb;
        std::memset(&reconFrames, 0, sizeof(oapv_frms_t));

        if (mIsRec && item->recon && !item->recon->graphicBlocks().empty()) {
            auto rBlock = item->recon->graphicBlocks()[0];
            mapBlockToImgb(rBlock, &reconImgb, codecDepth);

            reconFrames.num_frms = 1;
            reconFrames.frm[FRM_IDX].imgb = &reconImgb;
        }

        oapv_bitb_t bitb;
        std::memset(&bitb, 0, sizeof(oapv_bitb_t));
        bitb.addr = mBitstreamBuf;
        bitb.bsize = MAX_BS_BUF;

        oapve_stat_t stat;
        std::memset(&stat, 0, sizeof(oapve_stat_t));

        /* Update metadata for this frame if needed */
        Qv2Status metaStatus = updateMetadata();
        if (metaStatus != QV2_OK) {
            QV2_LOGW("Failed to update metadata: %d", metaStatus);
        }

        int ret = oapve_encode(mEncoderId, &inputFrames, mMetaDataId, &bitb,
                               &stat, mIsRec ? &reconFrames : nullptr);

        if (OAPV_FAILED(ret)) {
            QV2_LOGE("oapve_encode failed: %d", ret);
            item->result = QV2_ERR_INTERNAL;
        } else {
            if (item->output && !item->output->linearBlocks().empty()) {
                auto outBlock = item->output->linearBlocks()[0];
                if (outBlock->capacity() >= (size_t) stat.write) {
                    std::memcpy(outBlock->data(), bitb.addr, stat.write);
                    outBlock->setSize(stat.write);
                    item->result = QV2_OK;
                } else {
                    item->result = QV2_ERR_BUFFER_OVERFLOW;
                }
            } else {
                item->result = QV2_ERR_INVALID_ARG;
            }
        }

        if (mMetaDataId) {
            oapvm_rem_all(mMetaDataId);
        }
    }

    if (mListener) {
        mListener->onWorkDone(weak_from_this(), std::move(items));
    }

    return status;
}

Qv2Status Qv2ApvEncoder::start() {
    QV2_LOGV("start() entry.");

    if (mState != CONFIGURED) {
        QV2_LOGW("start() failed: Invalid state %s, expected CONFIGURED",
                 Qv2Component::stateToString(mState).c_str());
        return QV2_ERR_INTERNAL;
    }

    if (mBitstreamBuf) {
        delete[] mBitstreamBuf;
    }
    mBitstreamBuf = new unsigned char[MAX_BS_BUF];
    if (mBitstreamBuf == nullptr) {
        QV2_LOGE("cannot allocate bitstream buffer, size=%u", MAX_BS_BUF);
        return QV2_ERR_INTERNAL;
    }

    int ret = OAPV_OK;
    /* create encoder */
    mEncoderId = oapve_create(mCodecDesc.get(), &ret);
    if (mEncoderId == nullptr) {
        QV2_LOGE("cannot create APV encoder, ret=%d", ret);
        return QV2_ERR_INTERNAL;
    }

    /* create metadata */
    mMetaDataId = oapvm_create(&ret);
    if (mMetaDataId == nullptr) {
        QV2_LOGE("cannot create APV metadata, ret=%d", ret);
        return QV2_ERR_INTERNAL;
    }

    /* set encoder extra config (e.g., hash flag) */
    if (mUseHash) {
        int value = 1;
        int size = 4;
        ret = oapve_config(mEncoderId, OAPV_CFG_SET_USE_FRM_HASH, &value, &size);
        if (OAPV_FAILED(ret)) {
            QV2_LOGE("failed to set config for using frame hash, ret=%d", ret);
            return QV2_ERR_INTERNAL;
        }
    }

    setState(RUNNING);
    return QV2_OK;
}

Qv2Status Qv2ApvEncoder::stop() {
    QV2_LOGV("stop() entry.");

    if (mState != RUNNING) {
        QV2_LOGW("stop() failed: Invalid state %s, expected RUNNING",
                 Qv2Component::stateToString(mState).c_str());
        return QV2_ERR_INTERNAL;
    }

    setState(STOPPED);
    return QV2_OK;
}

Qv2Status Qv2ApvEncoder::flush() {
    QV2_LOGV("flush() entry.");
    return QV2_OK;
}

void Qv2ApvEncoder::showEncoderParams(oapve_cdesc_t *cdsc) const {
    QV2_LOGV("=== APV Encoder Configuration ===");
    QV2_LOGV("  Threads: %d", cdsc->threads);
    QV2_LOGV("  Max BS Buffer Size: %u", cdsc->max_bs_buf_size);

    for (int i = 0; i < cdsc->max_num_frms; i++) {
        const oapve_param_t &p = cdsc->param[i];
        QV2_LOGV("  Frame [%d] Config:", i);
        QV2_LOGV("    Profile IDC: %d, Level IDC: %d, Band IDC: %d",
                 p.profile_idc, p.level_idc, p.band_idc);
        QV2_LOGV("    Resolution: %dx%d", p.w, p.h);
        QV2_LOGV("    FPS: %d/%d", p.fps_num, p.fps_den);
        QV2_LOGV("    RC Type: %d, Bitrate: %d kbps, Filler: %d",
                 p.rc_type, p.bitrate, p.use_filler);
        QV2_LOGV("    QP: %u, QP Offset: (C1:%d, C2:%d, C3:%d)",
                 p.qp, p.qp_offset_c1, p.qp_offset_c2, p.qp_offset_c3);
        QV2_LOGV("    Use Q Matrix: %d", p.use_q_matrix);
        QV2_LOGV("    Tile Size: %dx%d", p.tile_w, p.tile_h);
        QV2_LOGV("    Preset: %d", p.preset);
        QV2_LOGV("    Color Desc Present: %d", p.color_description_present_flag);
        if (p.color_description_present_flag) {
            QV2_LOGV("    Primaries: %u, Transfer: %u, Matrix: %u, Full Range: %d",
                     p.color_primaries, p.transfer_characteristics,
                     p.matrix_coefficients, p.full_range_flag);
        }
    }
    QV2_LOGV("==================================");
}

int Qv2ApvEncoder::getCodecBitDepth(int profile_idc) const {
    switch (profile_idc) {
        case OAPV_PROFILE_422_10:
        case OAPV_PROFILE_400_10:
        case OAPV_PROFILE_444_10:
        case OAPV_PROFILE_4444_10:
            return 10;
        case OAPV_PROFILE_422_12:
        case OAPV_PROFILE_444_12:
        case OAPV_PROFILE_4444_12:
            return 12;
        default:
            return 0;
    }
    return 0;
}

void Qv2ApvEncoder::onStateChanged(State state) {
    QV2_LOGV("state changed to: %s", Qv2Component::stateToString(state).c_str());
}

void Qv2ApvEncoder::onRelease() {
    QV2_LOGV("onRelease() entry.");
    if (mEncoderId) {
        oapve_delete(mEncoderId);
        mEncoderId = nullptr;
    }
    if (mBitstreamBuf) {
        delete[] mBitstreamBuf;
        mBitstreamBuf = nullptr;
    }
    if (mMetaDataId) {
        oapvm_delete(mMetaDataId);
        mMetaDataId = nullptr;
    }
    mIsRec = false;
}

void Qv2ApvEncoder::mapBlockToImgb(const std::shared_ptr <Qv2Block2D> &block, oapv_imgb_t *imgb,
                                   int bitDepth) const {
    std::memset(imgb, 0, sizeof(oapv_imgb_t));
    imgb->cs = OAPV_CS_SET(mColorFmt, bitDepth, 0);
    imgb->np = block->numPlanes();

    int w = block->width();
    int h = block->height();
    imgb->w[0] = w;
    imgb->h[0] = h;

    if (imgb->np > 1) {
        int wShift = (mColorFmt == OAPV_CF_YCBCR420 ||
                      mColorFmt == OAPV_CF_YCBCR422 ||
                      mColorFmt == OAPV_CF_PLANAR2) ? 1 : 0;
        int hShift = (mColorFmt == OAPV_CF_YCBCR420) ? 1 : 0;

        for (int i = 1; i < imgb->np; i++) {
            imgb->w[i] = (w + wShift) >> wShift;
            imgb->h[i] = (h + hShift) >> hShift;
        }
    }

    for (uint32_t i = 0; i < (uint32_t) imgb->np; i++) {
        imgb->a[i] = block->addr(i);
        imgb->s[i] = block->stride(i);
        imgb->e[i] = block->elevation(i);
        imgb->aw[i] = ALIGN_VAL(imgb->w[i], OAPV_MB_W);
        imgb->ah[i] = ALIGN_VAL(imgb->h[i], OAPV_MB_H);
    }
}

int Qv2ApvEncoder::toOapvFmt(int qv2Format) const {
    switch (qv2Format) {
        case 0: // 400 (monochrome)
            return OAPV_CF_YCBCR400;
        case 2: // 422
            return OAPV_CF_YCBCR422;
        case 3: // 444
            return OAPV_CF_YCBCR444;
        case 4: // 4444
            return OAPV_CF_YCBCR4444;
        case 5: // P2 (Planar Y, Combined CbCr, 422)
            return OAPV_CF_PLANAR2;
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
            QV2_LOGW("Unknown Qv2ColorFormat %d, defaulting to YUV422(%d)", qv2Format, OAPV_CF_YCBCR422);
            return OAPV_CF_YCBCR422;
    }
}
