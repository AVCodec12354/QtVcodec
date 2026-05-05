#pragma once

#include <Qv2Source.h>
#include <fstream>
#include <memory>

class YUVSource : public Qv2Source {
public:
    YUVSource() : Qv2Source() {};

    std::shared_ptr<Qv2Buffer> getBuffer() override;
protected:
    int64_t calculateTotalFrame() override;
    void calculatePlaneSize() override;
};