#ifndef Y4MEXTRACTOR_H
#define Y4MEXTRACTOR_H

#include <iostream>
#include <cstdio>
#include <string>
#include "../ui/helpers/QTLogger.h"
#include "../codecs/apv/oapv_app_y4m.h"

class Y4MExtractor {
public:
    ~Y4MExtractor() = default;

    void setFile(std::string filePath);
    oapv_imgb_t* getBuffer();
    long getTotalFrame() { return mTotalFrame; }
    y4m_params_t getY4MParam() { return y4m_params; }

private:
    // Replace FILE *file by unique_ptr with deleter is fclose
    std::unique_ptr<FILE, decltype(&std::fclose)> mFile{nullptr, &std::fclose};
    long mFileSize;
    long mTotalFrame = 0;
    y4m_params_t y4m_params;

    void updateY4MParams();
    bool isHeaderExists();
    long calculateTotalFrame();
};

#endif // Y4MEXTRACTOR_H
