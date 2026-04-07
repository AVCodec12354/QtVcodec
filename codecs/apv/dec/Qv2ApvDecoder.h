#ifndef QV2APVDECODER_H
#define QV2APVDECODER_H

#include "../../bases/Qv2Component.h"
#include "oapv.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

class Qv2ApvDecoder : public Qv2Component {
public:
    enum State {
        STATE_UNINITIALIZED,
        STATE_INITIALIZED,
        STATE_RUNNING,
        STATE_STOPPED,
        STATE_ERROR
    };

    Qv2ApvDecoder();
    ~Qv2ApvDecoder() override;

    // Qv2Component interface
    void setListener(Listener* listener) override { mListener = listener; }
    bool queue(std::unique_ptr<Qv2Work> work) override;
    bool start() override;
    void stop() override;
    void flush() override;
    void release() override;

    // APV Specific setup
    int initialize(oapvd_cdesc_t* cdesc);
    State getState() const { return mState; }

private:
    void processLoop();
    void setState(State state);
    
    // OpenAPV Handles
    oapvd_t mHandler = nullptr;
    
    // Threading
    std::thread mThread;
    std::mutex mMutex;
    std::condition_variable mCv;
    std::queue<std::unique_ptr<Qv2Work>> mWorkQueue;
    
    std::atomic<State> mState{STATE_UNINITIALIZED};
    Listener* mListener = nullptr;
};

#endif // QV2APVDECODER_H
