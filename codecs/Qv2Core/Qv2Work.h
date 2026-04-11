#ifndef QV2WORK_H
#define QV2WORK_H

#include "Qv2Buffer.h"
#include <memory>

/**
 * @brief Qv2Work: A single unit of work for a component.
 */
struct Qv2Work {
    std::shared_ptr<Qv2Buffer> input;
    std::shared_ptr<Qv2Buffer> output;
    int result = 0;
    size_t processedSize = 0;
};

#endif // QV2WORK_H
