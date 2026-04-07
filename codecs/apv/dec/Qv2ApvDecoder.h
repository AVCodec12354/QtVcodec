#ifndef QV2APVDECODER_H
#define QV2APVDECODER_H

#include "../../bases/Qv2Component.h"
#include "oapv.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

class Qv2ApvDecoder : public Qv2Component {
public:
    Qv2ApvDecoder();
    ~Qv2ApvDecoder() override;

    // Qv2Component interface
    bool queue(std::unique_ptr<Qv2Work> work) override;
    bool start() override;
    void stop() override;
    void flush() override;
    void release() override;

    // APV Specific setup
    int initialize(oapvd_cdesc_t* cdesc);

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
};

#endif // QV2APVDECODER_H
