#include <gtest/gtest.h>
#include <YUVSource.h>

using YUVTestParam = std::tuple<std::string, int, int, int, int, Qv2ColorFormat>;

class YUVSourceTest : public ::testing::TestWithParam<YUVTestParam> {
public:
    void SetUp() override {}
    void TearDown() override {}
    YUVSource yuvSource;
};

TEST_P(YUVSourceTest, yuvSource) {
    auto[fileName, w, h, bitDepth, expectedFrame, colorFormat] = GetParam();
    std::string filePath = std::string(TEST_DATA_PATH) + fileName;
    std::cout << "Testing: " << filePath << std::endl;
    yuvSource.setDataSource(filePath.c_str(),w,h, bitDepth, colorFormat);

    EXPECT_EQ(yuvSource.getWidth(), w) << "Wrong width, expected is: " << w;
    EXPECT_EQ(yuvSource.getHeight(), h) << "Wrong height, expected is: " << h;
    EXPECT_EQ(yuvSource.getTotalFrame(), expectedFrame) << "Wrong totalFrame, expected is: " << expectedFrame;
}

INSTANTIATE_TEST_SUITE_P(
        YUV_AllFormats,
        YUVSourceTest,
        ::testing::Values(
            std::make_tuple("input_480x856_yuv420p.yuv", 480, 856, 8, 10, QV2FormatYUV420Planar),
            // 10-bit Semi-Planar & Packed
            std::make_tuple("pattern1_480x360_p010_10b.yuv", 480, 360, 10, 10, QV2FormatYUV420SemiPlanar),
            std::make_tuple("pattern1_320x240_y210_10b.yuv", 320, 240, 10, 5,  QV2FormatYUV422PackedPlanar),
            // 8-bit Semi-Planar (NV12, NV21)
            std::make_tuple("input_176x144_nv12_8b.yuv",     176, 144, 8,  3,  QV2FormatYUV420SemiPlanar),
            std::make_tuple("input_176x144_nv21_8b.yuv",     176, 144, 8,  3,  QV2FormatYUV420SemiPlanar),
            // 8-bit Packed (YUYV, UYVY)
            std::make_tuple("input_320x240_yuyv_8b.yuv",     320, 240, 8,  5,  QV2FormatYUV422PackedPlanar),
            std::make_tuple("input_320x240_uyvy_8b.yuv",     320, 240, 8,  5,  QV2FormatYUV422PackedPlanar)
        )
);