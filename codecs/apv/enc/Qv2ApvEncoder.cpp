#include "Qv2ApvEncoder.h"
#include "../../bases/Qv2Errors.h"
#include <iostream>

Qv2ApvEncoder::Qv2ApvEncoder() {
    setName("Qv2ApvEncoder");
    mState = STATE_UNINITIALIZED;
}

Qv2ApvEncoder::~Qv2ApvEncoder() {
    stop();
    if (mHandler) {
        oapve_delete(mHandler);
        mHandler = nullptr;
    }
}

void Qv2ApvEncoder::setState(State state) {
    mState = state;
}

int Qv2ApvEncoder::initialize(oapve_param_t* param) {
    if (mState != STATE_UNINITIALIZED) return OAPV_ERR;

    oapve_cdesc_t cdesc = {0};
    cdesc.threads = OAPV_CDESC_THREADS_AUTO;
    cdesc.max_bs_buf_size = 20 * 1024 * 1024; // 20MB default
    cdesc.param[0] = *param;

    int err;
    mHandler = oapve_create(&cdesc, &err);
    if (err == OAPV_OK) {
        setState(STATE_INITIALIZED);
    } else {
        setState(STATE_ERROR);
    }
    return err;
}

bool Qv2ApvEncoder::queue(std::unique_ptr<Qv2Work> work) {
    if (mState != STATE_RUNNING) return false;
    
    std::unique_lock<std::mutex> lock(mMutex);
    mWorkQueue.push(std::move(work));
    mCv.notify_one();
    return true;
}

bool Qv2ApvEncoder::start() {
    if (mState != STATE_INITIALIZED && mState != STATE_STOPPED) return false;
    
    setState(STATE_RUNNING);
    mThread = std::thread(&Qv2ApvEncoder::processLoop, this);
    return true;
}

void Qv2ApvEncoder::stop() {
    if (mState != STATE_RUNNING) return;
    
    mState = STATE_STOPPED;
    mCv.notify_all();
    if (mThread.joinable()) {
        mThread.join();
    }
}

void Qv2ApvEncoder::flush() {
    std::unique_lock<std::mutex> lock(mMutex);
    while (!mWorkQueue.empty()) {
        mWorkQueue.pop();
    }
}

void Qv2ApvEncoder::processLoop() {
    while (mState == STATE_RUNNING) {
        std::unique_ptr<Qv2Work> work;
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mCv.wait(lock, [this] { return !mWorkQueue.empty() || mState != STATE_RUNNING; });
            
            if (mState != STATE_RUNNING) break;
            work = std::move(mWorkQueue.front());
            mWorkQueue.pop();
        }

        if (!work || !work->input) {
            if (mListener) mListener->onError(this, QV2_ERR_INVALID_ARG);
            continue;
        }

        Qv2Buffer* inputBuffer = work->input.get();
        
        oapv_frms_t ifrms;
        ifrms.num_frms = 1;
        oapv_imgb_t* imgb = (oapv_imgb_t*)inputBuffer->plane(0).addr; 
        ifrms.frm[0].imgb = imgb;
        ifrms.frm[0].pbu_type = OAPV_PBU_TYPE_PRIMARY_FRAME;

        Qv2Buffer* outputBuffer = work->output.get();
        if (!outputBuffer) {
             if (mListener) mListener->onError(this, QV2_ERR_INVALID_ARG);
             continue;
        }
        
        oapv_bitb_t bitb;
        bitb.addr = outputBuffer->data();
        bitb.bsize = (int)outputBuffer->capacity();

        oapve_stat_t stat;
        int ret = oapve_encode(mHandler, &ifrms, nullptr, &bitb, &stat, nullptr);
        
        work->result = (ret == OAPV_OK) ? QV2_OK : QV2_ERR_INTERNAL;
        work->processedSize = stat.write;

        if (mListener) {
            mListener->onWorkDone(this, std::move(work));
        }
    }
}
