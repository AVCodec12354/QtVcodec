#ifndef Y4MEXTRACTOR_H
#define Y4MEXTRACTOR_H

#include <iostream>
#include <cstdio>
#include <string>
#include "QTLogger.h"
#include "../readers/oapv_app_y4m.h"

class Y4MExtractor {
public:
    Y4MExtractor() : file(nullptr) {};
    ~Y4MExtractor() {
        if (file != nullptr) {
            fclose(file);
            file = nullptr;
        }
    };

    void setFile(std::string filePath);
    oapv_imgb_t* getBuffer();
    int getTotalFrame() { return m_total_frame; }
    y4m_params_t getY4MParam() { return y4m_params; }

private:
    FILE  *file;
    long fileSize;
    long m_total_frame = 0;
    y4m_params_t y4m_params;

    void updateY4MParams();
    bool isHeaderExists();
    int calculateTotalFrame();
};

#endif // Y4MEXTRACTOR_H
