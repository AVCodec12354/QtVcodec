#include "Y4MSource.h"
#include <string>
#include <vector>

#define Y4M_HEADER_START "YUV4MPEG2"
#define Y4M_HEADER_START_SIZE 9

void Y4MSource::setDataSource(const std::string &filePath, int width, int height,
                              int bitDepth, Qv2ColorFormat colorFormat,
                              Qv2ColorPrimaries colorPrimaries,
                              Qv2ColorTransfer colorTransfer,
                              Qv2ColorMatrix colorMatrix,
                              Qv2ColorRange colorRange) {
    YUVSource::setDataSource(filePath, width, height, bitDepth, colorFormat);

    if (isY4M()) {
        std::cout << "Detected Y4M file, skipping header" << std::endl;
        skipFileHeader();
    } else {
        std::cout << "Not a valid Y4M file, resetting to pos 0" << std::endl;
        mFile.seekg(0, std::ios::beg);
    }
}

std::shared_ptr<Qv2Buffer> Y4MSource::getBuffer() {
    if (!mFile.is_open()) {
        std::cout << "File is not open" << std::endl;
        return nullptr;
    }
    mFile.ignore(mFrameMarkerSize);
    return YUVSource::getBuffer();
}

bool Y4MSource::isY4M() {
    char magic[Y4M_HEADER_START_SIZE];
    if (!mFile.read(magic, Y4M_HEADER_START_SIZE)) return false;
    bool result = (std::memcmp(magic, Y4M_HEADER_START, Y4M_HEADER_START_SIZE) == 0);
    mFile.seekg(0, std::ios::beg);
    return result;
}

void Y4MSource::skipFileHeader() {
    std::string dummy;
    if (std::getline(mFile, dummy)) {
        std::cout << "Skipping header: " << dummy << std::endl;
    } else {
        std::cout << "Failed to read header" << std::endl;
    }
}

int64_t Y4MSource::calculateTotalFrame() {
    if (!mFile.is_open()) {
        std::cout << "File is not open" << std::endl;
        return 0;
    }
    int bytesPerSample = (mBitDepth > 8) ? 2 : 1;
    int64_t frameSize =  0;
    for (int i = 0; i < mNumOfPlane; i++) { frameSize += mPlaneInfo[i].getSize(); }
    int64_t frameDataSize = static_cast<int64_t>(frameSize * bytesPerSample) + mFrameMarkerSize;
    if (frameDataSize <= 0) return 0;

    std::streampos currentPos = mFile.tellg();
    mFile.seekg(0, std::ios::end);
    int64_t fileSize = static_cast<int64_t>(mFile.tellg());
    mFile.seekg(currentPos, std::ios::beg);

    return (fileSize / frameDataSize);
}