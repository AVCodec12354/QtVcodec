#pragma once

#include <YUVSource.h>

class Y4MSource : public YUVSource {
public:
    Y4MSource() : YUVSource() {};

    void setDataSource(const std::string &filePath, int width, int height, int bitDepth,
                       Qv2ColorFormat colorFormat,
                       Qv2ColorPrimaries colorPrimaries,
                       Qv2ColorTransfer colorTransfer,
                       Qv2ColorMatrix colorMatrix,
                       Qv2ColorRange colorRange) override;

    std::shared_ptr<Qv2Buffer> getBuffer() override;
protected:
    int64_t calculateTotalFrame() override;
private:
    bool isY4M();
    void skipFileHeader();
    const int mFrameMarkerSize = 6;
};