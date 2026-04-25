#ifndef QV2WORK_H
#define QV2WORK_H

#include "Qv2Buffer.h"
#include <memory>

enum Qv2WorkFlags : uint32_t {
    QV2_WORK_FLAG_NONE = 0,
    QV2_WORK_FLAG_EOS = (1 << 0), // End of Stream
};

/**
 * @brief Qv2Work: A single unit of work for a component.
 */
struct Qv2Work {
    std::shared_ptr<Qv2Buffer> input;
    std::shared_ptr<Qv2Buffer> output;
    std::shared_ptr<Qv2Buffer> recon;
    uint64_t timestamp = 0;
    uint32_t flags = QV2_WORK_FLAG_NONE;
    int result = 0;
};

#endif // QV2WORK_H
