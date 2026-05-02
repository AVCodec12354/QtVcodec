#include <gtest/gtest.h>
#include <Qv2Source.h>
#include <YUVSource.h>
#include <Y4MSource.h>

using TestParam = std::tuple<std::string, std::string, int, int, int, int, Qv2ColorFormat>;

class Qv2SourceTest : public ::testing::TestWithParam<TestParam> {
public:
    void SetUp() override {}
    void TearDown() override {}
    std::unique_ptr<Qv2Source> source;
};

TEST_P(Qv2SourceTest, rawSource) {
    auto[type, fileName, w, h, bitDepth, expectedFrame, colorFormat] = GetParam();
    if (type == "yuv") {
        source = std::make_unique<YUVSource>();
    } else if (type == "y4m") {
        source = std::make_unique<Y4MSource>();
    }

    std::string filePath = std::string(TEST_DATA_PATH) + fileName;
    std::cout << "Testing: " << filePath << std::endl;
    source->setDataSource(filePath.c_str(),w,h, bitDepth, colorFormat);

    EXPECT_EQ(source->getWidth(), w) << "Wrong width, expected is: " << w;
    EXPECT_EQ(source->getHeight(), h) << "Wrong height, expected is: " << h;
    EXPECT_EQ(source->getTotalFrame(), expectedFrame) << "Wrong totalFrame, expected is: " << expectedFrame;

    auto buffer = source->getBuffer();
    EXPECT_GE(buffer->graphicBlocks().size(), 0) << "Expected size > 0";
    EXPECT_NE(buffer->graphicBlocks().at(0), nullptr) << "Expected buffer is NULL";
    for (int i = 0; i < buffer->graphicBlocks().at(0)->numPlanes(); i++) {
        EXPECT_NE(buffer->graphicBlocks().at(0)->addr(i), nullptr) << "Expected Plane " << i << " is NULL";
    }
}

INSTANTIATE_TEST_SUITE_P(
        YUV_AllFormats,
        Qv2SourceTest,
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
        Qv2SourceTest,
        ::testing::Values(
            // 10-bit Planar
            std::make_tuple("y4m", "pattern1_320x240_yuv420p10le_10bit_25fps.y4m", 320, 240, 10, 10, QV2_CF_YCBCR420_10LE),
            std::make_tuple("y4m", "pattern1_480x360_yuv444p10le_10bit_25fps.y4m", 480, 360, 10, 5,  QV2_CF_YCBCR444_10LE),
            // 8-bit Planar
            std::make_tuple("y4m", "input_320x240_yuv444p_8bit_25fps.y4m",         320, 240, 8,  8,  QV2_CF_YCBCR444),
            std::make_tuple("y4m", "input_480x360_yuv422p_8bit_25fps.y4m",         480, 360, 8,  5,  QV2_CF_YCBCR422)
        )
);