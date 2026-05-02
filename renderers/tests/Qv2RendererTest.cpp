#include <gtest/gtest.h>

#include <QApplication>
#include <QThread>

#include <Qv2Source.h>
#include <YUVSource.h>
#include <Y4MSource.h>
#include <VideoGLWidget.h>

using TestParam = std::tuple<std::string, std::string, int, int, int, int, Qv2ColorFormat>;

class Qv2RendererTest : public ::testing::TestWithParam<TestParam> {
public:
    void SetUp() override {
        mWidget = new VideoGLWidget();
    }
    void TearDown() override {
        if (mWidget) {
            mWidget->makeCurrent();
            delete mWidget;
            mWidget = nullptr;
        }
        mSource.reset();
        QCoreApplication::processEvents();
    }

    std::unique_ptr<Qv2Source> mSource;
    VideoGLWidget *mWidget = nullptr;
};

TEST_P(Qv2RendererTest, render) {
    auto[type, fileName, w, h, bitDepth, expectedFrame, colorFormat] = GetParam();
    if (type == "yuv") {
        mSource = std::make_unique<YUVSource>();
    } else if (type == "y4m") {
        mSource = std::make_unique<Y4MSource>();
    }
    std::string filePath = std::string(TEST_DATA_PATH) + fileName;
    std::cout << "Testing: " << filePath << std::endl;
    mSource->setDataSource(filePath.c_str(),w,h, bitDepth, colorFormat);
    auto buffer = mSource->getBuffer();
    EXPECT_NE(buffer, nullptr) << "Expected buffer is NULL";

    mWidget->resize(w, h);
    mWidget->show();
    for (int i = 0; i < expectedFrame; ++i) {
        buffer = mSource->getBuffer();
        if (buffer) {
            mWidget->bindBuffer(buffer);
            QCoreApplication::processEvents();
        }
        mWidget->makeCurrent();
        EXPECT_EQ(glGetError(), GL_NO_ERROR) << "Renderer error!";
        mWidget->doneCurrent();
        QThread::msleep(500);
    }
}

INSTANTIATE_TEST_SUITE_P(
        YUV_AllFormats,
        Qv2RendererTest,
        ::testing::Values(
            // 10-bit Semi-Planar & Packed
            std::make_tuple("yuv", "pattern1_480x360_p010le_10bit_25fps.yuv", 480, 360, 10, 10, QV2_CF_P010),
            std::make_tuple("yuv", "pattern1_320x240_y210le_10bit_25fps.yuv", 320, 240, 10, 5,  QV2_CF_Y210),
            // 8-bit Semi-Planar (NV12, NV21)
            std::make_tuple("yuv", "input_176x144_nv12_8bit_25fps.yuv",     176, 144, 8,  3,  QV2_CF_NV12),
            std::make_tuple("yuv", "input_176x144_nv21_8bit_25fps.yuv",     176, 144, 8,  3,  QV2_CF_NV21),
            // 8-bit Packed (YUYV, UYVY)
            std::make_tuple("yuv", "input_320x240_yuyv422_8bit_25fps.yuv",     320, 240, 8,  5,  QV2_CF_YUY2),
            std::make_tuple("yuv", "input_320x240_uyvy422_8bit_25fps.yuv",     320, 240, 8,  5,  QV2_CF_UYVY)
        )
);

INSTANTIATE_TEST_SUITE_P(
        Y4M_AllFormats,
        Qv2RendererTest,
        ::testing::Values(
            // 10-bit Planar
            std::make_tuple("y4m", "pattern1_320x240_yuv420p10le_10bit_25fps.y4m", 320, 240, 10, 10, QV2_CF_YCBCR420_10LE),
            std::make_tuple("y4m", "pattern1_480x360_yuv444p10le_10bit_25fps.y4m", 480, 360, 10, 5,  QV2_CF_YCBCR444_10LE),
            // 8-bit Planar
            std::make_tuple("y4m", "input_320x240_yuv444p_8bit_25fps.y4m",         320, 240, 8,  8,  QV2_CF_YCBCR444),
            std::make_tuple("y4m", "input_480x360_yuv422p_8bit_25fps.y4m",         480, 360, 8,  5,  QV2_CF_YCBCR422)
        )
);