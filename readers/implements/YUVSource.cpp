#include "YUVSource.h"

std::shared_ptr<Qv2Buffer> YUVSource::getBuffer() {
    int bytesPerSample = (mBitDepth > 8) ? 2 : 1;
    auto block = std::make_shared<Qv2Block2D>(mWidth, mHeight, mColorFormat, mBitDepth);
    for (int i = 0; i < mNumOfPlane; ++i) {
        size_t planeSizeInBytes = mPlaneInfo[i].getSize() * bytesPerSample;
        uint8_t *planeData = new uint8_t[planeSizeInBytes];
        size_t bytesRead = fread(planeData, 1, planeSizeInBytes, mFile.get());
        if (bytesRead == planeSizeInBytes) {
            int stride = mPlaneInfo[i].getWidth() * bytesPerSample;
            block->setPlane(i, planeData, stride, mPlaneInfo[i].getHeight());
        } else {
            delete[] planeData;
            return nullptr;
        }
    }
    return Qv2Buffer::CreateGraphicBuffer(block);
}

void YUVSource::calculatePlaneSize() {
    if (mWidth <= 0 || mHeight <= 0) return;
    switch (mColorFormat) {
        case QV2FormatYUV420Flexible:
        case QV2FormatYUV420PackedPlanar:
        case QV2FormatYUV420Planar:
            mNumOfPlane = 3;
            mPlaneInfo[PLANE_Y].config(mWidth, mHeight);
            mPlaneInfo[PLANE_U].config(mWidth/2, mHeight/2);
            mPlaneInfo[PLANE_V].config(mWidth/2, mHeight/2);
            break;
        case QV2FormatYUV420PackedSemiPlanar:
        case QV2FormatYUV420SemiPlanar:
        case QV2FormatYUVP010:
            mNumOfPlane = 2;
            mPlaneInfo[PLANE_Y].config(mWidth, mHeight);
            mPlaneInfo[PLANE_UV].config(mWidth, mHeight/2);
            break;
        case QV2FormatYUV422Flexible:
        case QV2FormatYUV422PackedPlanar:
        case QV2FormatYUV422Planar:
            mNumOfPlane = 3;
            mPlaneInfo[PLANE_Y].config(mWidth, mHeight);
            mPlaneInfo[PLANE_U].config(mWidth/2, mHeight);
            mPlaneInfo[PLANE_V].config(mWidth/2, mHeight);
            break;
        case QV2FormatYUV422PackedSemiPlanar:
        case QV2FormatYUV422SemiPlanar:
        case QV2FormatYUVP210:
            mNumOfPlane = 2;
            mPlaneInfo[PLANE_Y].config(mWidth, mHeight);
            mPlaneInfo[PLANE_UV].config(mWidth, mHeight);
            break;
        case QV2FormatYUV444Flexible:
            mNumOfPlane = 3;
            mPlaneInfo[PLANE_Y].config(mWidth, mHeight);
            mPlaneInfo[PLANE_U].config(mWidth, mHeight);
            mPlaneInfo[PLANE_V].config(mWidth, mHeight);
            break;
        case QV2FormatYUV444Interleaved:
            mNumOfPlane = 1;
            mPlaneInfo[PLANE_Y].config(mWidth * 3, mHeight);
            break;
        default: // YUV 420 Planar
            mNumOfPlane = 3;
            mPlaneInfo[PLANE_Y].config(mWidth, mHeight);
            mPlaneInfo[PLANE_U].config(mWidth/2, mHeight/2);
            mPlaneInfo[PLANE_V].config(mWidth/2, mHeight/2);
            break;
    }
}

int64_t YUVSource::calculateTotalFrame() {
    if (mFile == nullptr) {
        std::cout << "mFile is nullptr" << std::endl;
        return 0;
    }
    int bytesPerSample = (mBitDepth > 8) ? 2 : 1;
    int64_t frameSize =  0;
    for (int i = 0; i < mNumOfPlane; i++) { frameSize += mPlaneInfo[i].getSize(); }
    int64_t frameDataSize = static_cast<int64_t>(frameSize * bytesPerSample);
// Lấy kích thước file (Hỗ trợ file > 2GB)
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