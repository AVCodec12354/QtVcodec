#ifndef QV2COMPONENT_H
#define QV2COMPONENT_H

#include "Qv2Work.h"
#include "Qv2Errors.h"
#include "Qv2Params.h"
#include <memory>
#include <string>
#include <atomic>
#include <vector>

class Qv2Component : public std::enable_shared_from_this<Qv2Component> {
public:
    enum State {
        UNINITIALIZED,
        INITIALIZED,
        CONFIGURED,
        STARTING,
        RUNNING,
        FLUSHING,
        PAUSED,
        STOPPING,
        STOPPED,
        RELEASING,
        ERROR
    };

    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void onWorkDone(std::weak_ptr<Qv2Component> component, std::unique_ptr<Qv2Work> work) = 0;
        virtual void onError(std::weak_ptr<Qv2Component> component, int error) = 0;
        virtual void onStateChanged(std::weak_ptr<Qv2Component> component, State newState) {}
    };

    virtual ~Qv2Component() = default;

    void setName(const std::string& name) { mName = name; }
    std::string getName() const { return mName; }
    State getState() const { return mState; }
    
    void setState(State state) { 
        mState = state; 
        if (auto self = weak_from_this().lock()) {
            if (mListener) {
                mListener->onStateChanged(self, state);
            }
        }
        onStateChanged(state);
    }

    void setListener(Listener* listener) { mListener = listener; }

    virtual int configure(const std::vector<Qv2Param*>& params) = 0;
    virtual int query(std::vector<Qv2Param*>& params) const = 0;
    virtual int queue(std::vector<std::unique_ptr<Qv2Work>> items) = 0;
    
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual void flush() = 0;
    
    void release() {
        if (mState == RUNNING) stop();
        flush();
        setState(RELEASING);
        onRelease();
        setState(UNINITIALIZED);
    }

protected:
    virtual void onStateChanged(State state);
    virtual void onRelease() = 0;

    std::string mName;
    Listener* mListener = nullptr;
    std::atomic<State> mState{UNINITIALIZED};
};

#endif // QV2COMPONENT_H
