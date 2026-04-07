#include "Qv2ApvDecoder.h"
#include "../../bases/Qv2Errors.h"
#include <iostream>

Qv2ApvDecoder::Qv2ApvDecoder() {
    setName("Qv2ApvDecoder");
    mState = STATE_UNINITIALIZED;
}

Qv2ApvDecoder::~Qv2ApvDecoder() {
    release();
}

void Qv2ApvDecoder::setState(State state) {
    mState = state;
}

int Qv2ApvDecoder::initialize(oapvd_cdesc_t* cdesc) {
    if (mState != STATE_UNINITIALIZED) return OAPV_ERR;

    int err;
    mHandler = oapvd_create(cdesc, &err);
    if (err == OAPV_OK) {
        setState(STATE_INITIALIZED);
    } else {
        setState(STATE_ERROR);
    }
    return err;
}

bool Qv2ApvDecoder::queue(std::unique_ptr<Qv2Work> work) {
    if (mState != STATE_RUNNING) return false;
    
    std::unique_lock<std::mutex> lock(mMutex);
    mWorkQueue.push(std::move(work));
    mCv.notify_one();
    return true;
}

bool Qv2ApvDecoder::start() {
    if (mState != STATE_INITIALIZED && mState != STATE_STOPPED) return false;
    
    setState(STATE_RUNNING);
    mThread = std::thread(&Qv2ApvDecoder::processLoop, this);
    return true;
}

void Qv2ApvDecoder::stop() {
    if (mState == STATE_RUNNING) {
        mState = STATE_STOPPED;
        mCv.notify_all();
        if (mThread.joinable()) {
            mThread.join();
        }
    }
}

void Qv2ApvDecoder::flush() {
    std::unique_lock<std::mutex> lock(mMutex);
    while (!mWorkQueue.empty()) {
        mWorkQueue.pop();
    }
}

void Qv2ApvDecoder::release() {
    stop();
    if (mHandler) {
        oapvd_delete(mHandler);
        mHandler = nullptr;
    }
    setState(STATE_UNINITIALIZED);
}

void Qv2ApvDecoder::processLoop() {
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

        if (work->input->type() != Qv2BufferType::BUFFER_1D) {
            if (mListener) mListener->onError(this, QV2_ERR_BAD_FORMAT);
            continue;
        }
        Qv2Buffer1D* input1D = static_cast<Qv2Buffer1D*>(work->input.get());

        oapv_bitb_t bitb;
        bitb.addr = input1D->data();
        bitb.ssize = (int)input1D->size();

        if (!work->output || work->output->type() != Qv2BufferType::BUFFER_2D) {
             if (mListener) mListener->onError(this, QV2_ERR_INVALID_ARG);
             continue;
        }
        Qv2Buffer2D* output2D = static_cast<Qv2Buffer2D*>(work->output.get());
        
        oapv_frms_t ofrms;
        ofrms.num_frms = 1;
        oapv_imgb_t* imgb = (oapv_imgb_t*)output2D->plane(0).addr; 
        ofrms.frm[0].imgb = imgb;

        oapvd_stat_t stat;
        int ret = oapvd_decode(mHandler, &bitb, &ofrms, nullptr, &stat);
        
        work->result = (ret == OAPV_OK) ? QV2_OK : QV2_ERR_INTERNAL;
        work->processedSize = stat.read;

        if (mListener) {
            mListener->onWorkDone(this, std::move(work));
        }
    }
}
