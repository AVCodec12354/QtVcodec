#include "Qv2ApvEncoder.h"
#include "Qv2Log.h"
#include <cstring>

#define LOG_TAG "Qv2ApvEncoder"

constexpr char COMPONENT_NAME[] = "qv2.apv.encoder";
constexpr uint32_t kMinOutBufferSize = 524288;
constexpr uint32_t kMaxBitstreamBufSize = 16 * 1024 * 1024;
constexpr int32_t kApvQpMin = 0;
constexpr int32_t kApvQpMax = 51;
constexpr int32_t kApvDefaultQP = 32;

#define PROFILE_APV_DEFAULT 0
#define LEVEL_APV_DEFAULT 0
#define MAX_NUM_FRMS (1)

Qv2ApvEncoder::Qv2ApvEncoder()
        : mEncoderId(nullptr), mMetaDataId(nullptr), mBitstreamBuf(nullptr) {

    setState(UNINITIALIZED);

    QV2_LOGI("Constructor called.");
    mName = COMPONENT_NAME;
    mCodecDesc = std::make_unique<oapve_cdesc_t>();

    std::memset(mCodecDesc.get(), 0, sizeof(oapve_cdesc_t));
    std::memset(&mInputFrames, 0, sizeof(oapv_frms_t));
    std::memset(&mReconFrames, 0, sizeof(oapv_frms_t));

    mCodecDesc->max_bs_buf_size = kMaxBitstreamBufSize;
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

    /* create encoder */
    mEncoderId = oapve_create(mCodecDesc.get(), NULL);
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
        mReconFrames.frm[i].imgb = nullptr;
    }

    mBitstreamBuf = new unsigned char[kMaxBitstreamBufSize];
    if (mBitstreamBuf == nullptr) {
        QV2_LOGE("cannot allocate bitstream buffer, size=%u", kMaxBitstreamBufSize);
        return;
    }

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
    QV2_LOGI("configure() called with %zu parameters.", params.size());

    if (mState != UNINITIALIZED && mState != INITIALIZED && mState != CONFIGURED) {
        QV2_LOGW("configure() failed: Invalid state %d", static_cast<int>(mState.load()));
        return QV2_ERR_INTERNAL;
    }

    setState(CONFIGURED);
    return QV2_OK;
}

Qv2Status Qv2ApvEncoder::query(std::vector<Qv2Param*>& params) const {
    QV2_LOGD("query() called.");
    return QV2_OK;
}

Qv2Status Qv2ApvEncoder::queue(std::vector<std::unique_ptr<Qv2Work>> items) {
    QV2_LOGD("queue() called with %zu items.", items.size());
    return QV2_OK;
}

Qv2Status Qv2ApvEncoder::start() {
    QV2_LOGI("start() called.");
    setState(RUNNING);
    return QV2_OK;
}

Qv2Status Qv2ApvEncoder::stop() {
    QV2_LOGI("stop() called.");
    setState(STOPPED);
    return QV2_OK;
}

Qv2Status Qv2ApvEncoder::flush() {
    QV2_LOGD("flush() called.");
    return QV2_OK;
}

void Qv2ApvEncoder::onStateChanged(State state) {
    QV2_LOGI("state changed to: %d", static_cast<int>(state));
}

void Qv2ApvEncoder::onRelease() {
    QV2_LOGI("onRelease() called.");
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
