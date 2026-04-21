#include "Qv2ApvEncoder.h"
#include "Qv2Log.h"
#include <cstring>

#define LOG_TAG "Qv2ApvEncoder"

#define MAX_BS_BUF   (128 * 1024 * 1024)
#define MAX_NUM_FRMS (1)           // supports only 1-frame in an access unit
#define FRM_IDX      (0)           // supports only 1-frame in an access unit
#define MAX_NUM_CC   (OAPV_MAX_CC) // Max number of color components (upto 4:4:4:4)

constexpr char COMPONENT_NAME[] = "qv2.apv.encoder";

Qv2ApvEncoder::Qv2ApvEncoder()
        : mEncoderId(nullptr), mMetaDataId(nullptr), mBitstreamBuf(nullptr) {

    setState(UNINITIALIZED);

    QV2_LOGI("Constructor called.");
    mName = COMPONENT_NAME;
    mCodecDesc = std::make_unique<oapve_cdesc_t>();

    std::memset(mCodecDesc.get(), 0, sizeof(oapve_cdesc_t));
    std::memset(&mInputFrames, 0, sizeof(oapv_frms_t));
    if(mIsRec)
        std::memset(&mReconFrames, 0, sizeof(oapv_frms_t));

    mCodecDesc->max_bs_buf_size = MAX_BS_BUF;
    mCodecDesc->max_num_frms = MAX_NUM_FRMS;
    mCodecDesc->threads = OAPV_CDESC_THREADS_AUTO;

    int ret = OAPV_OK;
    for (int32_t i = 0; i < MAX_NUM_FRMS; i++) {
        oapve_param_t* param = &mCodecDesc->param[i];
        ret = oapve_param_default(param);
        if (OAPV_FAILED(ret)) {
            QV2_LOGE("cannot set default parameter");
        }
    }
    showEncoderParams(mCodecDesc.get());
    setState(INITIALIZED);
}

Qv2ApvEncoder::~Qv2ApvEncoder() {
    QV2_LOGI("Destructor called.");
    release();
}

std::string Qv2ApvEncoder::getVersion() const {
    unsigned int ver_num;
    const char* ver_str = oapv_version(&ver_num);
    return std::string("OpenAPV ") + ver_str;
}

Qv2Status Qv2ApvEncoder::configure(const std::vector<Qv2Param*>& params) {
    QV2_LOGI("configure() entry. Params size: %zu", params.size());

    if (mState != INITIALIZED && mState != STOPPED) {
        QV2_LOGW("configure() failed: Invalid state %s", 
                 Qv2Component::stateToString(mState).c_str());
        return QV2_ERR_INTERNAL;
    }

    oapve_param_t* p = &mCodecDesc->param[FRM_IDX];

    for (auto param : params) {
        if (!param) continue;

        switch (param->mId) {
            case Qv2VideoSizeInput::ID: {
                auto v = static_cast<Qv2VideoSizeInput*>(param);
                p->w = v->mWidth;
                p->h = v->mHeight;
                QV2_LOGD("Set VideoSize: %dx%d", p->w, p->h);
                break;
            }
            case Qv2BitrateSetting::ID: {
                auto v = static_cast<Qv2BitrateSetting*>(param);
                p->bitrate = v->mBitrate / 1000; // bps to kbps
                p->rc_type = OAPV_RC_ABR;
                QV2_LOGD("Set Bitrate: %d kbps", p->bitrate);
                break;
            }
            case Qv2FrameRateInput::ID: {
                auto v = static_cast<Qv2FrameRateInput*>(param);
                p->fps_num = static_cast<int>(v->mFps);
                p->fps_den = 1;
                QV2_LOGD("Set FPS: %d", p->fps_num);
                break;
            }
            case Qv2BitDepthInput::ID: {
                auto v = static_cast<Qv2BitDepthInput*>(param);
//                QV2_LOGD("Set BitDepth: %d", p->bit_depth);
                break;
            }
            case Qv2ColorFormatInput::ID: {
                auto v = static_cast<Qv2ColorFormatInput*>(param);
                // chroma_format_idc is inferred from profile in OpenAPV
                QV2_LOGD("ColorFormat (IDC) requested: %d", v->mColorFormat);
                break;
            }
            case Qv2ProfileOutput::ID: {
                auto v = static_cast<Qv2ProfileOutput*>(param);
                p->profile_idc = v->mProfile;
                QV2_LOGD("Set Profile: %d", p->profile_idc);
                break;
            }
            case Qv2LevelOutput::ID: {
                auto v = static_cast<Qv2LevelOutput*>(param);
                p->level_idc = v->mLevel;
                QV2_LOGD("Set Level: %d", p->level_idc);
                break;
            }
            case Qv2BandOutput::ID: {
                auto v = static_cast<Qv2BandOutput*>(param);
                p->band_idc = v->mBand;
                QV2_LOGD("Set Band: %d", p->band_idc);
                break;
            }
            case Qv2QPInput::ID: {
                auto v = static_cast<Qv2QPInput*>(param);
                p->qp = static_cast<unsigned char>(v->mQP);
                p->rc_type = OAPV_RC_CQP;
                QV2_LOGD("Set QP: %d", (int)p->qp);
                break;
            }
            default:
                QV2_LOGW("Unknown param ID: 0x%08X", param->mId);
                break;
        }
    }

    showEncoderParams(mCodecDesc.get());

    setState(CONFIGURED);
    return QV2_OK;
}

Qv2Status Qv2ApvEncoder::query(std::vector<Qv2Param*>& params) const {
    QV2_LOGD("query() entry.");
    return QV2_OK;
}

Qv2Status Qv2ApvEncoder::queue(std::vector<std::unique_ptr<Qv2Work>> items) {
    QV2_LOGD("queue() entry. Items size: %zu", items.size());
    return QV2_OK;
}

Qv2Status Qv2ApvEncoder::start() {
    QV2_LOGI("start() entry.");

    mBitb.addr = mBitstreamBuf;
    mBitb.bsize = MAX_BS_BUF;

    int ret = OAPV_OK;
    /* create encoder */
    mEncoderId = oapve_create(mCodecDesc.get(), &ret);
    if (mEncoderId == NULL) {
        QV2_LOGE("cannot create APV encoder");
    }

    /* create metadata */
    mMetaDataId = oapvm_create(&ret);
    if (mMetaDataId == NULL) {
        QV2_LOGE("cannot create APV metadata");
    }

    for (int32_t i = 0; i < MAX_NUM_FRMS; i++) {
        mInputFrames.frm[i].imgb = nullptr;
        if(mIsRec)
            mReconFrames.frm[i].imgb = nullptr;
    }

    mBitstreamBuf = new unsigned char[MAX_BS_BUF];
    if (mBitstreamBuf == nullptr) {
        QV2_LOGE("cannot allocate bitstream buffer, size=%u", MAX_BS_BUF);
        return QV2_ERR_INTERNAL;
    }
    setState(RUNNING);
    return QV2_OK;
}

Qv2Status Qv2ApvEncoder::stop() {
    QV2_LOGI("stop() entry.");
    setState(STOPPED);
    return QV2_OK;
}

Qv2Status Qv2ApvEncoder::flush() {
    QV2_LOGD("flush() entry.");
    return QV2_OK;
}

void Qv2ApvEncoder::showEncoderParams(oapve_cdesc_t* cdsc) const {
    QV2_LOGI("=== APV Encoder Configuration ===");
    QV2_LOGI("  Threads: %d", cdsc->threads);
    QV2_LOGI("  Max BS Buffer Size: %u", cdsc->max_bs_buf_size);
    
    for (int i = 0; i < cdsc->max_num_frms; i++) {
        const oapve_param_t& p = cdsc->param[i];
        QV2_LOGI("  Frame [%d] Config:", i);
        QV2_LOGI("    Profile IDC: %d, Level IDC: %d, Band IDC: %d", 
                 p.profile_idc, p.level_idc, p.band_idc);
        QV2_LOGI("    Resolution: %dx%d", p.w, p.h);
        QV2_LOGI("    FPS: %d/%d", p.fps_num, p.fps_den);
        QV2_LOGI("    RC Type: %d, Bitrate: %d kbps, Filler: %d", 
                 p.rc_type, p.bitrate, p.use_filler);
        QV2_LOGI("    QP: %u, QP Offset: (C1:%d, C2:%d, C3:%d)", 
                 p.qp, p.qp_offset_c1, p.qp_offset_c2, p.qp_offset_c3);
        QV2_LOGI("    Use Q Matrix: %d", p.use_q_matrix);
        QV2_LOGI("    Tile Size: %dx%d", p.tile_w, p.tile_h);
        QV2_LOGI("    Preset: %d", p.preset);
        QV2_LOGI("    Color Desc Present: %d", p.color_description_present_flag);
        if (p.color_description_present_flag) {
            QV2_LOGI("    Primaries: %u, Transfer: %u, Matrix: %u, Full Range: %d", 
                     p.color_primaries, p.transfer_characteristics, 
                     p.matrix_coefficients, p.full_range_flag);
        }
    }
    QV2_LOGI("==================================");
}

void Qv2ApvEncoder::onStateChanged(State state) {
    QV2_LOGI("state changed to: %s", Qv2Component::stateToString(state).c_str());
}

void Qv2ApvEncoder::onRelease() {
    QV2_LOGI("onRelease() entry.");
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
}
