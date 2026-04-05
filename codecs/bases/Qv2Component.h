#ifndef QV2COMPONENT_H
#define QV2COMPONENT_H

#include "Qv2Work.h"
#include <memory>
#include <string>

/**
 * @brief Qv2Component: Abstract base class for all codec components (APV, H.264, etc.)
 */
class Qv2Component {
public:
    /**
     * @brief Listener interface for asynchronous callbacks.
     */
    class Listener {
    public:
        virtual ~Listener() = default;
        
        /**
         * @brief Called when a work unit is completed.
         * @param component The component that finished the work.
         * @param work The completed work unit containing output data.
         */
        virtual void onWorkDone(Qv2Component* component, std::unique_ptr<Qv2Work> work) = 0;
        
        /**
         * @brief Called when an error occurs during processing.
         */
        virtual void onError(Qv2Component* component, int error) = 0;
    };

    virtual ~Qv2Component() = default;

    /**
     * @brief Set the name of the component.
     */
    virtual void setName(const std::string& name) { mName = name; }

    /**
     * @brief Get the name of the component.
     */
    virtual std::string getName() const { return mName; }

    /**
     * @brief Sets the listener for receiving callbacks.
     */
    virtual void setListener(Listener* listener) = 0;

    /**
     * @brief Queues a work unit for processing (non-blocking).
     * @return true if the work was successfully queued.
     */
    virtual bool queue(std::unique_ptr<Qv2Work> work) = 0;

    /**
     * @brief Starts the component processing thread.
     */
    virtual bool start() = 0;

    /**
     * @brief Stops the component and joins the processing thread.
     */
    virtual void stop() = 0;

    /**
     * @brief Flushes all pending work units in the queue.
     */
    virtual void flush() = 0;

protected:
    std::string mName;
};

#endif // QV2COMPONENT_H
