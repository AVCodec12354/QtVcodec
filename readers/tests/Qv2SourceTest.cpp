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
}

INSTANTIATE_TEST_SUITE_P(
        YUV_AllFormats,
        Qv2SourceTest,
        ::testing::Values(
            std::make_tuple("yuv", "input_480x856_yuv420p.yuv", 480, 856, 8, 10, QV2FormatYUV420Planar),
            // 10-bit Semi-Planar & Packed
            std::make_tuple("yuv", "pattern1_480x360_p010_10b.yuv", 480, 360, 10, 10, QV2FormatYUV420SemiPlanar),
            std::make_tuple("yuv", "pattern1_320x240_y210_10b.yuv", 320, 240, 10, 5,  QV2FormatYUV422PackedPlanar),
            // 8-bit Semi-Planar (NV12, NV21)
            std::make_tuple("yuv", "input_176x144_nv12_8b.yuv",     176, 144, 8,  3,  QV2FormatYUV420SemiPlanar),
            std::make_tuple("yuv", "input_176x144_nv21_8b.yuv",     176, 144, 8,  3,  QV2FormatYUV420SemiPlanar),
            // 8-bit Packed (YUYV, UYVY)
            std::make_tuple("yuv", "input_320x240_yuyv_8b.yuv",     320, 240, 8,  5,  QV2FormatYUV422PackedPlanar),
            std::make_tuple("yuv", "input_320x240_uyvy_8b.yuv",     320, 240, 8,  5,  QV2FormatYUV422PackedPlanar)
        )
);

INSTANTIATE_TEST_SUITE_P(
        Y4M_AllFormats,
        Qv2SourceTest,
        ::testing::Values(
            std::make_tuple("y4m", "pattern1_yuv422p10le_320x240_25fps.y4m", 320, 240, 10, 125, QV2FormatYUV422Planar),
            // 10-bit Planar
            std::make_tuple("y4m", "pattern1_320x240_p420_10b.y4m", 320, 240, 10, 10, QV2FormatYUV420Planar),
            std::make_tuple("y4m", "pattern1_480x360_p444_10b.y4m", 480, 360, 10, 5,  QV2FormatYUV444Flexible),
            // 8-bit Planar
            std::make_tuple("y4m", "input_320x240_p444_8b.y4m",     320, 240, 8,  8,  QV2FormatYUV444Flexible),
            std::make_tuple("y4m", "input_480x360_p422_8b.y4m",     480, 360, 8,  5,  QV2FormatYUV422Planar)
        )
);