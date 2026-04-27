#include "YUVSource.h"

std::shared_ptr<Qv2Buffer> YUVSource::getBuffer() {
    if (!mFile.is_open()) {
        std::cout << "File is not open" << std::endl;
        return nullptr;
    }
    int bytesPerSample = (mBitDepth > 8) ? 2 : 1;
    auto block =
            std::make_shared<Qv2Block2D>(mWidth, mHeight, mColorFormat, mBitDepth);
    for (int i = 0; i < mNumOfPlane; ++i) {
        const size_t planeSize = static_cast<size_t>(mPlaneInfo[i].getSize()) * bytesPerSample;
        const uint32_t stride = mPlaneInfo[i].getWidth() * bytesPerSample;
        const uint32_t elevation = mPlaneInfo[i].getHeight();

        uint8_t* rawData = new (std::nothrow) uint8_t[planeSize];
        if (!rawData) return nullptr;

        if (!mFile.read(reinterpret_cast<char*>(rawData), planeSize)
            || static_cast<size_t>(mFile.gcount()) != planeSize
        ) {
            delete[] rawData;
            return nullptr;
        }
        block->setPlane(i, rawData, stride, elevation);
    }
    return Qv2Buffer::CreateGraphicBuffer(block);
}

void YUVSource::calculatePlaneSize() {
    if (mWidth <= 0 || mHeight <= 0) return;
    const int halfWidth = (mWidth + 1) / 2;
    const int halfHeight = (mHeight + 1) / 2;

    switch (mColorFormat) {
        case QV2FormatYUV420Flexible:
        case QV2FormatYUV420PackedPlanar:
        case QV2FormatYUV420Planar:
            mNumOfPlane = 3;
            mPlaneInfo[PLANE_Y].config(mWidth, mHeight);
            mPlaneInfo[PLANE_U].config(halfWidth, halfHeight);
            mPlaneInfo[PLANE_V].config(halfWidth, halfHeight);
            break;

        case QV2FormatYUV420PackedSemiPlanar:
        case QV2FormatYUV420SemiPlanar:
        case QV2FormatYUVP010:
            mNumOfPlane = 2;
            mPlaneInfo[PLANE_Y].config(mWidth, mHeight);
            mPlaneInfo[PLANE_UV].config(mWidth, halfHeight);
            break;

        case QV2FormatYUV422Flexible:
        case QV2FormatYUV422PackedPlanar:
        case QV2FormatYUV422Planar:
            mNumOfPlane = 3;
            mPlaneInfo[PLANE_Y].config(mWidth, mHeight);
            mPlaneInfo[PLANE_U].config(halfWidth, mHeight);
            mPlaneInfo[PLANE_V].config(halfWidth, mHeight);
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

        default:
            mNumOfPlane = 3;
            mPlaneInfo[PLANE_Y].config(mWidth, mHeight);
            mPlaneInfo[PLANE_U].config(halfWidth, halfHeight);
            mPlaneInfo[PLANE_V].config(halfWidth, halfHeight);
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

    const std::streampos currentPos = mFile.tellg();
    if (currentPos == std::streampos(-1)) return 0;
    mFile.seekg(0, std::ios::end);
    const int64_t fileSize = static_cast<int64_t>(mFile.tellg());
    mFile.seekg(currentPos, std::ios::beg);

    if (fileSize <= 0) return 0;
    return (fileSize / frameDataSize);
}