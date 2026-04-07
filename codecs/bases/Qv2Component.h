#ifndef QV2COMPONENT_H
#define QV2COMPONENT_H

#include "Qv2Work.h"
#include "Qv2Errors.h"
#include <memory>
#include <string>
#include <atomic>

/**
 * @brief Qv2Component: Abstract base class for all codec components.
 */
class Qv2Component : public std::enable_shared_from_this<Qv2Component> {
public:
    enum State {
        STATE_UNINITIALIZED,
        STATE_INITIALIZED,
        STATE_RUNNING,
        STATE_STOPPED,
        STATE_ERROR
    };

    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void onWorkDone(std::weak_ptr<Qv2Component> component, std::unique_ptr<Qv2Work> work) = 0;
        virtual void onError(std::weak_ptr<Qv2Component> component, int error) = 0;
    };

    virtual ~Qv2Component() = default;

    virtual void setName(const std::string& name) { mName = name; }
    virtual std::string getName() const { return mName; }
    virtual State getState() const { return mState; }

    virtual void setListener(Listener* listener) { mListener = listener; }

    virtual bool queue(std::unique_ptr<Qv2Work> work) = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual void flush() = 0;
    virtual void release() = 0;

protected:
    std::string mName;
    Listener* mListener = nullptr;
    std::atomic<State> mState{STATE_UNINITIALIZED};
};

#endif // QV2COMPONENT_H
