#include "Qv2ApvEncoder.h"

#define LOG_TAG "Qv2ApvEncoder"

#include "Qv2Log.h"
#include "../../../readers/oapv_app_util.h"
#include <cstring>
#include <limits>

#define MAX_BS_BUF   (128 * 1024 * 1024)
#define MAX_NUM_FRMS (1)           // supports only 1-frame in an access unit
#define FRM_IDX      (0)           // supports only 1-frame in an access unit
constexpr char COMPONENT_NAME[] = "qv2.apv.encoder";

namespace {

using OwnedImage = std::unique_ptr<oapv_imgb_t, decltype(&imgb_release)>;

OwnedImage makeOwnedImage(int width, int height, int cs) {
    return OwnedImage(imgb_create(width, height, cs), &imgb_release);
}

int getCodecBitDepthFromProfile(int profileIdc) {
    switch (profileIdc) {
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
}

Qv2Status mapOapvStatus(int status) {
    switch (status) {
        case OAPV_OK:
            return QV2_OK;
        case OAPV_ERR_INVALID_ARGUMENT:
            return QV2_ERR_INVALID_ARG;
        case OAPV_ERR_OUT_OF_MEMORY:
            return QV2_ERR_NO_MEMORY;
        case OAPV_ERR_UNSUPPORTED:
        case OAPV_ERR_INVALID_PROFILE:
        case OAPV_ERR_INVALID_LEVEL:
        case OAPV_ERR_INVALID_FAMILY:
            return QV2_ERR_UNSUPPORTED;
        case OAPV_ERR_UNSUPPORTED_COLORSPACE:
            return QV2_ERR_BAD_FORMAT;
        case OAPV_ERR_INVALID_WIDTH:
        case OAPV_ERR_INVALID_HEIGHT:
        case OAPV_ERR_INVALID_QP:
            return QV2_ERR_BAD_VALUE;
        case OAPV_ERR_OUT_OF_BS_BUF:
            return QV2_ERR_BUFFER_OVERFLOW;
        case OAPV_ERR_MALFORMED_BITSTREAM:
            return QV2_ERR_MALFORMED;
        default:
            return QV2_ERR_INTERNAL;
    }
}

uint32_t getExpectedPlaneCount(uint32_t format) {
    switch (format) {
        case OAPV_CF_YCBCR400:
            return 1;
        case OAPV_CF_PLANAR2:
            return 2;
        case OAPV_CF_YCBCR420:
        case OAPV_CF_YCBCR422:
        case OAPV_CF_YCBCR422W:
        case OAPV_CF_YCBCR444:
            return 3;
        case OAPV_CF_YCBCR4444:
            return 4;
        default:
            return 0;
    }
}

Qv2Status validateInputBuffer(const Qv2Buffer2D& source) {
    if (source.getWidth() == 0 || source.getHeight() == 0) {
        return QV2_ERR_BAD_VALUE;
    }

    const uint32_t expectedPlanes = getExpectedPlaneCount(source.getFormat());
    if (expectedPlanes == 0) {
        return QV2_ERR_BAD_FORMAT;
    }

    if (source.getNumPlanes() < expectedPlanes) {
        return QV2_ERR_BAD_FORMAT;
    }

    for (uint32_t plane = 0; plane < expectedPlanes; ++plane) {
        if (!source.getAddr(plane) || source.getStride(plane) == 0) {
            return QV2_ERR_INVALID_ARG;
        }
    }

    return QV2_OK;
}

Qv2Status copyBuffer2DToImage(const Qv2Buffer2D& source, oapv_imgb_t* dest) {
    if (!dest) {
        return QV2_ERR_INVALID_ARG;
    }

    if (source.getNumPlanes() < static_cast<uint32_t>(dest->np)) {
        return QV2_ERR_BAD_FORMAT;
    }

    const size_t bytesPerSample = static_cast<size_t>(OAPV_CS_GET_BYTE_DEPTH(dest->cs));
    for (int plane = 0; plane < dest->np; ++plane) {
        auto* srcBase = source.getAddr(plane);
        if (!srcBase) {
            return QV2_ERR_INVALID_ARG;
        }

        const size_t rowBytes = static_cast<size_t>(dest->w[plane]) * bytesPerSample;
        const size_t srcStride = source.getStride(plane) ? source.getStride(plane) : rowBytes;
        auto* dstBase = static_cast<uint8_t*>(dest->a[plane]);

        for (int row = 0; row < dest->h[plane]; ++row) {
            std::memcpy(dstBase, srcBase, rowBytes);
            srcBase += srcStride;
            dstBase += dest->s[plane];
        }
    }

    return QV2_OK;
}

Qv2Status prepareInputImage(const Qv2Buffer2D& inputBuffer,
                            int codecBitDepth,
                            OwnedImage& ownedImage,
                            oapv_imgb_t*& image) {
    image = nullptr;

    const Qv2Status validateStatus = validateInputBuffer(inputBuffer);
    if (validateStatus != QV2_OK) {
        return validateStatus;
    }

    const int cs = OAPV_CS_SET(static_cast<int>(inputBuffer.getFormat()),
                               static_cast<int>(inputBuffer.getBitDepth()),
                               0);

    ownedImage = makeOwnedImage(static_cast<int>(inputBuffer.getWidth()),
                                static_cast<int>(inputBuffer.getHeight()),
                                cs);
    if (!ownedImage) {
        return QV2_ERR_NO_MEMORY;
    }

    const Qv2Status copyStatus = copyBuffer2DToImage(inputBuffer, ownedImage.get());
    if (copyStatus != QV2_OK) {
        return copyStatus;
    }

    image = ownedImage.get();

    const int inputFormat = OAPV_CS_GET_FORMAT(image->cs);
    const int inputBitDepth = OAPV_CS_GET_BIT_DEPTH(image->cs);
    if (codecBitDepth > 0 && inputBitDepth != codecBitDepth && inputFormat != OAPV_CF_PLANAR2) {
        auto normalized = makeOwnedImage(image->w[0],
                                         image->h[0],
                                         OAPV_CS_SET(inputFormat, codecBitDepth, OAPV_CS_GET_ENDIAN(image->cs)));
        if (!normalized) {
            return QV2_ERR_NO_MEMORY;
        }

        imgb_cpy(normalized.get(), image);
        ownedImage = std::move(normalized);
        image = ownedImage.get();
    }

    return QV2_OK;
}

} // namespace

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

    const auto& param = mCodecDesc->param[FRM_IDX];
    const int codecBitDepth = getCodecBitDepthFromProfile(param.profile_idc);
    if (codecBitDepth == 0) {
        QV2_LOGE("queue() failed: invalid profile %d", param.profile_idc);
        return QV2_ERR_BAD_FORMAT;
    }

    Qv2Status overallStatus = QV2_OK;
    for (auto& item : items) {
        if (!item) {
            overallStatus = overallStatus == QV2_OK ? QV2_ERR_INVALID_ARG : overallStatus;
            continue;
        }

        item->result = QV2_OK;
        if (!item->input || !item->output) {
            item->result = QV2_ERR_INVALID_ARG;
            overallStatus = overallStatus == QV2_OK ? QV2_ERR_INVALID_ARG : overallStatus;
            continue;
        }

        if (item->input->getType() != Qv2BufferType::BUFFER_2D) {
            QV2_LOGE("queue() failed: APV encoder only accepts Qv2Buffer2D input");
            item->result = QV2_ERR_BAD_FORMAT;
            overallStatus = overallStatus == QV2_OK ? QV2_ERR_BAD_FORMAT : overallStatus;
            continue;
        }

        if (item->output->getType() != Qv2BufferType::BUFFER_1D) {
            item->result = QV2_ERR_BAD_FORMAT;
            overallStatus = overallStatus == QV2_OK ? QV2_ERR_BAD_FORMAT : overallStatus;
            continue;
        }

        auto* outputBuffer = static_cast<Qv2Buffer1D*>(item->output.get());
        if (!outputBuffer->getData()) {
            item->result = QV2_ERR_INVALID_ARG;
            overallStatus = overallStatus == QV2_OK ? QV2_ERR_INVALID_ARG : overallStatus;
            continue;
        }

        if (outputBuffer->getCapacity() == 0 ||
            outputBuffer->getCapacity() > static_cast<size_t>(std::numeric_limits<int>::max())) {
            item->result = QV2_ERR_BUFFER_OVERFLOW;
            overallStatus = overallStatus == QV2_OK ? QV2_ERR_BUFFER_OVERFLOW : overallStatus;
            continue;
        }

        OwnedImage ownedInput(nullptr, &imgb_release);
        oapv_imgb_t* inputImage = nullptr;
        const auto& inputBuffer = static_cast<const Qv2Buffer2D&>(*item->input);
        const Qv2Status prepareStatus = prepareInputImage(inputBuffer, codecBitDepth, ownedInput, inputImage);
        if (prepareStatus != QV2_OK) {
            item->result = prepareStatus;
            overallStatus = overallStatus == QV2_OK ? prepareStatus : overallStatus;
            continue;
        }

        if (inputImage->w[0] != param.w || inputImage->h[0] != param.h) {
            QV2_LOGE("queue() failed: input image size %dx%d does not match configured size %dx%d",
                     inputImage->w[0], inputImage->h[0], param.w, param.h);
            item->result = QV2_ERR_BAD_VALUE;
            overallStatus = overallStatus == QV2_OK ? QV2_ERR_BAD_VALUE : overallStatus;
            continue;
        }

        outputBuffer->setSize(0);
        outputBuffer->mTimestamp = item->input->mTimestamp;
        outputBuffer->mFlags = item->input->mFlags;

        std::memset(&mInputFrames, 0, sizeof(mInputFrames));
        mInputFrames.num_frms = 1;
        mInputFrames.frm[FRM_IDX].imgb = inputImage;
        mInputFrames.frm[FRM_IDX].group_id = 1;
        mInputFrames.frm[FRM_IDX].pbu_type = OAPV_PBU_TYPE_PRIMARY_FRAME;

        mBitb.addr = outputBuffer->getData();
        mBitb.bsize = static_cast<int>(outputBuffer->getCapacity());
        mBitb.ssize = 0;
        mBitb.err = 0;

        const int ret = oapve_encode(mEncoderId,
                                     &mInputFrames,
                                     mMetaDataId,
                                     &mBitb,
                                     &mStat,
                                     mIsRec ? &mReconFrames : nullptr);
        if (OAPV_FAILED(ret)) {
            const Qv2Status status = mapOapvStatus(ret);
            QV2_LOGE("queue() failed: oapve_encode returned %d", ret);
            item->result = status;
            overallStatus = overallStatus == QV2_OK ? status : overallStatus;
            continue;
        }

        outputBuffer->setSize(static_cast<size_t>(mStat.write));
        item->result = QV2_OK;
        QV2_LOGD("Encoded 1 work item. Bitstream size: %d bytes", mStat.write);
    }

    if (mListener) {
        mListener->onWorkDone(weak_from_this(), std::move(items));
    }

    return overallStatus;
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

    int codec_depth = (mCodecDesc->param[0].profile_idc == OAPV_PROFILE_422_10 ||
                       mCodecDesc->param[0].profile_idc == OAPV_PROFILE_400_10 ||
                       mCodecDesc->param[0].profile_idc == OAPV_PROFILE_444_10 ||
                       mCodecDesc->param[0].profile_idc == OAPV_PROFILE_4444_10) ? 10 : (
                                                                                                mCodecDesc->param[0].profile_idc ==
                                                                                                OAPV_PROFILE_422_12 ||
                                                                                                mCodecDesc->param[0].profile_idc ==
                                                                                                OAPV_PROFILE_444_12 ||
                                                                                                mCodecDesc->param[0].profile_idc ==
                                                                                                OAPV_PROFILE_4444_12)
                                                                                        ? 12 : 0;

    if (codec_depth == 0) {
        QV2_LOGE("Invalid profile");
        return QV2_ERR_INTERNAL;
    }

    for (int32_t i = 0; i < MAX_NUM_FRMS; i++) {
//        mInputFrames.frm[i].imgb = imgb_create(mCodecDesc->param[0].w,
//                                               mCodecDesc->param[0].h,
//                                               OAPV_CS_SET(OAPV_CF_YCBCR422, 10, 0));
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
