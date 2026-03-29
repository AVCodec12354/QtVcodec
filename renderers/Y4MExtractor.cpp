#include "Y4MExtractor.h"
#include "VideoGLWidget.h"

void Y4MExtractor::setFile(std::string filePath) {
    if (file != nullptr) { fclose(file); }
    file = fopen(filePath.c_str(), "rb");
    updateY4MParams();
}

bool Y4MExtractor::isHeaderExists() {
    if (file == nullptr) { return false; }
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