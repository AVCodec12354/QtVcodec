#ifndef QV2APVENCODER_H
#define QV2APVENCODER_H

#include "../../bases/Qv2Component.h"
#include "oapv.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

class Qv2ApvEncoder : public Qv2Component {
public:
    Qv2ApvEncoder();
    ~Qv2ApvEncoder() override;

    // Qv2Component interface
    bool queue(std::unique_ptr<Qv2Work> work) override;
    bool start() override;
    void stop() override;
    void flush() override;
    void release() override;

    // APV Specific setup
    int initialize(oapve_param_t* param);

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
};

#endif // QV2APVENCODER_H
