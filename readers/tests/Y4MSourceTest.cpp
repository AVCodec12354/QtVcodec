#include <gtest/gtest.h>
#include <Y4MSource.h>

using Y4MTestParam = std::tuple<std::string, int, int, int, int, Qv2ColorFormat>;

class Y4MSourceTest : public ::testing::TestWithParam<Y4MTestParam> {
public:
    void SetUp() override {}
    void TearDown() override {}
    Y4MSource y4mSource;
};

TEST_P(Y4MSourceTest, y4mSource) {
    auto[fileName, w, h, bitDepth, expectedFrame, colorFormat] = GetParam();
    std::string filePath = std::string(TEST_DATA_PATH) + fileName;
    std::cout << "Testing: " << filePath << std::endl;

    y4mSource.setDataSource(filePath.c_str(),w,h, bitDepth, colorFormat);
    EXPECT_EQ(y4mSource.getWidth(), w) << "Wrong width, expected is: " << w;
    EXPECT_EQ(y4mSource.getHeight(), h) << "Wrong height, expected is: " << h;
    EXPECT_EQ(y4mSource.getTotalFrame(), expectedFrame) << "Wrong totalFrame, expected is: " << expectedFrame;
}

INSTANTIATE_TEST_SUITE_P(
        Y4M_AllFormats,
        Y4MSourceTest,
        ::testing::Values(
            std::make_tuple("pattern1_yuv422p10le_320x240_25fps.y4m", 320, 240, 10, 125, QV2FormatYUV422Planar),
            // 10-bit Planar
            std::make_tuple("pattern1_320x240_p420_10b.y4m", 320, 240, 10, 10, QV2FormatYUV420Planar),
            std::make_tuple("pattern1_480x360_p444_10b.y4m", 480, 360, 10, 5,  QV2FormatYUV444Flexible),
            // 8-bit Planar
            std::make_tuple("input_320x240_p444_8b.y4m",     320, 240, 8,  8,  QV2FormatYUV444Flexible),
            std::make_tuple("input_480x360_p422_8b.y4m",     480, 360, 8,  5,  QV2FormatYUV422Planar)
        )
);