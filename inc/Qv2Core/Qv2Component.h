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

    /**
     * @brief Utility function to convert State to human-readable string.
     */
    static inline std::string stateToString(State state) {
        switch (state) {
            case UNINITIALIZED: return "UNINITIALIZED";
            case INITIALIZED:   return "INITIALIZED";
            case CONFIGURED:    return "CONFIGURED";
            case STARTING:      return "STARTING";
            case RUNNING:       return "RUNNING";
            case FLUSHING:      return "FLUSHING";
            case PAUSED:        return "PAUSED";
            case STOPPING:      return "STOPPING";
            case STOPPED:       return "STOPPED";
            case RELEASING:     return "RELEASING";
            case ERROR:         return "ERROR";
            default:            return "UNKNOWN";
        }
    }

    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void onWorkDone(std::weak_ptr<Qv2Component> component,
                                std::vector<std::unique_ptr<Qv2Work>> workItems) = 0;
        virtual void onError(std::weak_ptr<Qv2Component> component,
                             Qv2Status error) = 0;
        virtual void onStateChanged(std::weak_ptr<Qv2Component> component,
                                    State newState) = 0;
    };

    virtual ~Qv2Component() = default;

    std::string getName() const { return mName; }
    State getState() const { return mState; }
    
    virtual std::string getVersion() const = 0;

    void setState(State state) { 
        mState = state; 
        if (auto self = weak_from_this().lock()) {
            if (mListener) {
                mListener->onStateChanged(self, state);
            }
        }
        onStateChanged(state);
    }

    void enableRecon(bool enable) {
        mIsRec = enable;
    }

    void setListener(Listener* listener) { mListener = listener; }

    virtual Qv2Status configure(const std::vector<Qv2Param*>& params) = 0;
    virtual Qv2Status query(std::vector<Qv2Param*>& params) const = 0;
    virtual Qv2Status queue(std::vector<std::unique_ptr<Qv2Work>> items) = 0;
    
    virtual Qv2Status start() = 0;
    virtual Qv2Status stop() = 0;
    virtual Qv2Status flush() = 0;
    
    void release() {
        if (mState == RUNNING) stop();
        flush();
        setState(RELEASING);
        onRelease();
        setState(UNINITIALIZED);
    }

protected:
    virtual void onStateChanged(State state) = 0;
    virtual void onRelease() = 0;

    std::string mName;
    Listener* mListener = nullptr;
    std::atomic<State> mState{UNINITIALIZED};
    bool mIsRec = false;
};

#endif // QV2COMPONENT_H
