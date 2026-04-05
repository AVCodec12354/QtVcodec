#ifndef QV2WORK_H
#define QV2WORK_H

#include "Qv2Buffer.h"
#include "Qv2Errors.h"
#include <memory>

/**
 * @brief Qv2Work: The atomic unit of work in the pipeline.
 * It carries the input and output buffers together through the component.
 */
struct Qv2Work {
    uint64_t index;
    
    std::unique_ptr<Qv2Buffer> input;
    std::unique_ptr<Qv2Buffer> output;

    int result = QV2_OK;
    size_t processedSize = 0;

    explicit Qv2Work(uint64_t idx) : index(idx) {};
};

#endif // QV2WORK_H
