#ifndef QV2APVENCODER_H
#define QV2APVENCODER_H

#include "../../bases/Qv2Component.h"
#include "oapv.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

class Qv2ApvEncoder : public Qv2Component {
public:
    enum State {
        STATE_UNINITIALIZED,
        STATE_INITIALIZED,
        STATE_RUNNING,
        STATE_STOPPED,
        STATE_ERROR
    };

    Qv2ApvEncoder();
    ~Qv2ApvEncoder() override;

    // Qv2Component interface
    void setListener(Listener* listener) override { mListener = listener; }
    bool queue(std::unique_ptr<Qv2Work> work) override;
    bool start() override;
    void stop() override;
    void flush() override;

    // APV Specific setup
    int initialize(oapve_param_t* param);
    State getState() const { return mState; }

private:
    void processLoop();
    void setState(State state);
    
    // OpenAPV Handles
    oapve_t mHandler = nullptr;
    
    // Threading
    std::thread mThread;
    std::mutex mMutex;
    std::condition_variable mCv;
    std::queue<std::unique_ptr<Qv2Work>> mWorkQueue;
    
    std::atomic<State> mState{STATE_UNINITIALIZED};
    Listener* mListener = nullptr;
};

#endif // QV2APVENCODER_H
