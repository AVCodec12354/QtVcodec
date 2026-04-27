#include "YUVSource.h"

std::shared_ptr<Qv2Buffer> YUVSource::getBuffer() {
    if (!mFile.is_open()) return nullptr;
    int bytesPerSample = (mBitDepth > 8) ? 2 : 1;
    auto block =
            std::make_shared<Qv2Block2D>(mWidth, mHeight, mColorFormat, mBitDepth);
    for (int i = 0; i < mNumOfPlane; ++i) {
        size_t planeSizeInBytes = static_cast<size_t>(mPlaneInfo[i].getSize()) * bytesPerSample;
        std::vector<uint8_t> planeData(planeSizeInBytes);
        mFile.read(reinterpret_cast<char*>(planeData.data()), planeSizeInBytes);
        if (mFile.gcount() == static_cast<std::streamsize>(planeSizeInBytes)) {
            int stride = mPlaneInfo[i].getWidth() * bytesPerSample;
            block->setPlane(i, planeData.data(), stride, mPlaneInfo[i].getHeight());
        } else {
            std::cout << "Read frame failed or EOF reached." << std::endl;
            return nullptr;
        }
    }
    mCurrentFrame++;
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
    if (!mFile.is_open()) {
        std::cout << "File is not open" << std::endl;
        return 0;
    }
    int bytesPerSample = (mBitDepth > 8) ? 2 : 1;
    int64_t frameSize = 0;
    for (int i = 0; i < mNumOfPlane; i++) { frameSize += mPlaneInfo[i].getSize(); }
    int64_t frameDataSize = frameSize * bytesPerSample;
    if (frameDataSize <= 0) return 0;

    std::streampos currentPos = mFile.tellg();
    mFile.seekg(0, std::ios::end);
    int64_t fileSize = static_cast<int64_t>(mFile.tellg());
    mFile.seekg(currentPos, std::ios::beg);

    return (fileSize / frameDataSize);
}