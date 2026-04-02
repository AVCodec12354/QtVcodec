#include "Y4MExtractor.h"
#include "VideoGLWidget.h"

void Y4MExtractor::setFile(std::string filePath) {
    if (file != nullptr) { fclose(file); }
    file = fopen(filePath.c_str(), "rb");
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    QTInfo("Y4MExtractor", "File size: " + std::to_string(fileSize));
    updateY4MParams();
    m_total_frame = calculateTotalFrame();
}

long Y4MExtractor::calculateTotalFrame() {
    double pixelMultiplier = 0;
    switch (y4m_params.color_format) {
        case OAPV_CF_YCBCR400: // Chỉ có kênh Y
            pixelMultiplier = 1.0;
            break;
        case OAPV_CF_YCBCR420: // Y + U(1/4) + V(1/4) = 1.5
            pixelMultiplier = 1.5;
            break;
        case OAPV_CF_YCBCR422:
        case OAPV_CF_YCBCR422W: // Y + U(1/2) + V(1/2) = 2.0
            pixelMultiplier = 2.0;
            break;
        case OAPV_CF_YCBCR444: // Y + U + V = 3.0
            pixelMultiplier = 3.0;
            break;
        case OAPV_CF_YCBCR4444: // Y + U + V + A = 4.0
            pixelMultiplier = 4.0;
            break;
        default:
            pixelMultiplier = 1.5;
            break;
    }
    int bytesPerPixel = (y4m_params.bit_depth > 8) ? 2 : 1;
    long frameDataSize = (long)(y4m_params.w * y4m_params.h * pixelMultiplier * bytesPerPixel);
    long afterHeaderPosition = ftell(file); // to get size of header => Call after read Y4M Header - updateY4MParams()
    long remainingSize = fileSize - afterHeaderPosition - 6; // 6 bytes for "FRAME\n"
    return remainingSize / frameDataSize;
}

bool Y4MExtractor::isHeaderExists() {
    if (file == nullptr) { return false; }
    fseek(file, 0, SEEK_SET);
    return (y4m_test(file) == 1);
}

void Y4MExtractor::updateY4MParams() {
    if (isHeaderExists()) {
        fseek(file, 0, SEEK_SET);
        if (y4m_header_parser(file, &y4m_params) == 0) {
            QTInfo("Y4MExtractor", "Got video info: " + std::to_string(y4m_params.w) + "x" +
                                   std::to_string(y4m_params.h) + ", FPS: " +
                                   std::to_string(y4m_params.fps_num) + "/" +
                                   std::to_string(y4m_params.fps_den) + ", ColorFormat: " +
                                   std::to_string(y4m_params.color_format));
        }
    } else {
        QTError("Y4MExtractor", "No Y4M header found");
    }
}

oapv_imgb_t* Y4MExtractor::getBuffer() {
    oapv_imgb_t *buffer = imgb_create(y4m_params.w, y4m_params.h, OAPV_CS_SET(y4m_params.color_format, y4m_params.bit_depth, 0));
    if (imgb_read(file, buffer, y4m_params.w, y4m_params.h, true) == -1) {
        QTInfo("Y4MExtractor", "Error reading Y4M file");
        imgb_release(buffer);
        return nullptr;
    }
    return buffer;
}