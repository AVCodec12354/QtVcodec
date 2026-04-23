#pragma once

#include "../interfaces/Qv2Source.h"

class YUVSource : public Qv2Source {
public:
    YUVSource() : Qv2Source() {};
    ~YUVSource() override = default;

    std::unique_ptr<Qv2Buffer> getBuffer() override;
protected:
    int64_t calculateTotalFrame() override;
    void calculatePlaneSize() override;
};