#pragma once

#include <YUVSource.h>

class Y4MSource : public YUVSource {
public:
    Y4MSource() : YUVSource() {
        if (isY4M()) {
            skipFileHeader();
        } else {
            fseek(mFile.get(), 0, SEEK_SET);
        }
    };

    std::shared_ptr<Qv2Buffer> getBuffer() override;
protected:
    int64_t calculateTotalFrame() override;
private:
    bool isY4M();
    void skipFileHeader();
};