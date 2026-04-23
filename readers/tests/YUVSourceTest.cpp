#include <gtest/gtest.h>
#include <YUVSource.h>
#include <Y4MSource.h>

class RawTest : public ::testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(RawTest, input_480x856_yuv420p) {
    /*
    $ ffprobe -v error -f rawvideo -video_size 480x856 -pixel_format yuv420p -count_frames -show_entries stream=nb_read_frames,pix_fmt,width,height -of default=noprint_wrappers=1 input_480x856_yuv420p.yuv
    width=480
    height=856
    pix_fmt=yuv420p
    nb_read_frames=10
     */
    std::string filePath = std::string(TEST_DATA_PATH) + "input_480x856_yuv420p.yuv";
    YUVSource yuvSource;
    int w = 480, h = 856, bitDepth = 8;
    int expectedFrame = 10;
    yuvSource.setDataSource(filePath.c_str(),w,h, bitDepth, QV2FormatYUV420Planar);

    EXPECT_EQ(yuvSource.getWidth(), w) << "Wrong width, expected is: " << w;
    EXPECT_EQ(yuvSource.getHeight(), h) << "Wrong height, expected is: " << h;
    EXPECT_EQ(yuvSource.getTotalFrame(), expectedFrame) << "Wrong totalFrame, expected is: " << expectedFrame;
}

TEST_F(RawTest, pattern1_yuv422p10le_320x240_25fps) {
    /*
    $ ffprobe -v error -select_streams v:0 -count_frames -show_entries stream=nb_read_frames,width,height,pix_fmt -of default=noprint_wrappers=1 pattern1_yuv422p10le_320x240_25fps.y4m
    width=320
    height=240
    pix_fmt=yuv422p10le
    nb_read_frames=125
     */
    std::string filePath = std::string(TEST_DATA_PATH) + "pattern1_yuv422p10le_320x240_25fps.y4m";
    Y4MSource y4mSource;
    int w = 320, h = 340, bitDepth = 10;
    int expectedFrame = 125;
    y4mSource.setDataSource(filePath.c_str(),w,h, bitDepth, QV2FormatYUV422Planar);

    EXPECT_EQ(y4mSource.getWidth(), w) << "Wrong width, expected is: " << w;
    EXPECT_EQ(y4mSource.getHeight(), h) << "Wrong height, expected is: " << h;
    EXPECT_EQ(y4mSource.getTotalFrame(), expectedFrame) << "Wrong totalFrame, expected is: " << expectedFrame;
}