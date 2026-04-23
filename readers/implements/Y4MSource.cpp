#include "Y4MSource.h"

std::shared_ptr<Qv2Buffer> Y4MSource::getBuffer() {
    char frameMarker[6];
    if (fread(frameMarker, 1, 6, mFile.get()) == 6) {
        if (strncmp(frameMarker, "FRAME", 5) != 0) {
            fseek(mFile.get(), -6, SEEK_CUR);
        }
        if (frameMarker[5] != '\n') {
            int c;
            while ((c = fgetc(mFile.get())) != '\n' && c != EOF);
        }
    }
    return YUVSource::getBuffer();
}

bool Y4MSource::isY4M() {
    char title[9];
    if (fread(title, 1, 9, mFile.get()) == 9) {
        return (strncmp(title, "YUV4MPEG2", 9) == 0);
    }
    return false;
}

void Y4MSource::skipFileHeader() {
    int c;
    while ((c = fgetc(mFile.get())) != '\n' && c != EOF);
}

int64_t Y4MSource::calculateTotalFrame() {
    if (mFile == nullptr) return 0;
    int bytesPerSample = (mBitDepth > 8) ? 2 : 1;
    int64_t frameSize =  0;
    for (int i = 0; i < mNumOfPlane; i++) { frameSize += mPlaneInfo[i].getSize(); }
    // 6 mean FRAME in raw file
    int64_t frameDataSize = static_cast<int64_t>(frameSize * bytesPerSample) + 6;

#ifdef _WIN32
    _fseeki64(mFile.get(), 0, SEEK_END);
    int64_t fileSize = _ftelli64(mFile.get());
    _fseeki64(mFile.get(), 0, SEEK_SET);
#else
    fseeko(mFile.get(), 0, SEEK_END);
    int64_t fileSize = ftello(mFile.get());
    fseeko(mFile.get(), 0, SEEK_SET);
#endif
    return (fileSize / frameDataSize);
}