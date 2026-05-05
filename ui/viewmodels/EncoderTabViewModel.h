#ifndef ENCODERVIEWMODEL_H
#define ENCODERVIEWMODEL_H

#include <iostream>
#include <memory>
#include <vector>
#include <thread>

#include <Qv2Constants.h>
#include <Qv2Component.h>
#include <Qv2ComponentFactory.h>
#include <Qv2Source.h>
#include <VideoGLWidget.h>
#include <QObject>

using namespace std;

class EncoderTabViewModel : public QObject {
    Q_OBJECT
public:
    EncoderTabViewModel(VideoGLWidget *glWidget,
                        QObject *parent = nullptr) : QObject(parent) {
        mVideoWidget = qobject_cast<VideoGLWidget*>(glWidget);
        qRegisterMetaType<std::shared_ptr<Qv2Buffer>>("std::shared_ptr<Qv2Buffer>");
    };
    ~EncoderTabViewModel();

    void start(const std::string &file);
    void stop();

    // Basic Settings:
    void setWidth(int value) { mWidth = value; }
    void setHeight(int value) { mHeight = value; }
    void setFPS(int value) { mFPS = value; }
    void setBitDepth(int value) { mBitDepth = value; }
    void setPixelFormat(string value);
    // Bitrate and Quality
    void enableBitrateABR(bool value) { mEnableBitrateMode = value; }
    void setQuantizationParameters(int value) { mQP = value; }
    void setProfile(string value);
    void setLevel(int value);
    void setFamily(string value);
    void setBand(int value) { mBand = value; }
    // Optimize
    void setMaxCU(int value) { mMaxCU = value; }
    void setSpeedCU(int value) { mSpeedCU = value; }
    void setWidthOfTile(int value) { mWidthOfTile = value; }
    void setHeightOfTile(int value) { mHeightOfTile = value; }
    // Color Metadata
    void setPrimaries(string value);
    void setTransfer(string value);
    void setMatrix(string value);
    void setRange(string value);
    void setMasteringDisplay(int value) { mMasteringDisplay = value; }
    void setContentLightLevel(int value) { mContentLightLevel = value; }

signals:
    void playing(long currentFrame, long totalFrame);
    void finished();

private:
    std::thread mRenderThread;
    std::atomic<bool> mIsRunning{false};
    std::shared_ptr<Qv2Source> mRawSource;
    VideoGLWidget* mVideoWidget;

    int mWidth, mHeight, mFPS, mBitDepth, mQP, mBand;
    int mMaxCU, mSpeedCU, mWidthOfTile, mHeightOfTile;
    int mMasteringDisplay, mContentLightLevel;
    bool mEnableBitrateMode = false;
    Qv2ColorFormat mPixelFormat;
    Qv2APVProfile mProfile;
    Qv2APVLevel mLevel;
    Qv2APVFamily mFamily;
    Qv2ColorAspect mColorAspect;
};

// ================================
static const unordered_map<string, Qv2ColorFormat> PixelFormatMap = {
        // Planar Formats (8-bit)
        {"YCbCr400", QV2_CF_YCBCR400},
        {"YCbCr420", QV2_CF_YCBCR420},
        {"YCbCr422", QV2_CF_YCBCR422},
        {"YCbCr444", QV2_CF_YCBCR444},
        {"YCbCr4444", QV2_CF_YCBCR4444},

        // Planar Formats (10-bit LE)
        {"YCbCr400_10LE", QV2_CF_YCBCR400_10LE},
        {"YCbCr420_10LE", QV2_CF_YCBCR420_10LE},
        {"YCbCr422_10LE", QV2_CF_YCBCR422_10LE},
        {"YCbCr444_10LE", QV2_CF_YCBCR444_10LE},
        {"YCbCr4444_10LE", QV2_CF_YCBCR4444_10LE},

        // Planar Formats (12-bit LE)
        {"YCbCr400_12LE", QV2_CF_YCBCR400_12LE},
        {"YCbCr420_12LE", QV2_CF_YCBCR420_12LE},
        {"YCbCr422_12LE", QV2_CF_YCBCR422_12LE},
        {"YCbCr444_12LE", QV2_CF_YCBCR444_12LE},
        {"YCbCr4444_12LE", QV2_CF_YCBCR4444_12LE},

        // Semi-Planar Formats
        {"NV12", QV2_CF_NV12},
        {"NV21", QV2_CF_NV21},
        {"NV16", QV2_CF_NV16},
        {"P010", QV2_CF_P010},
        {"P210", QV2_CF_P210},
        {"P012", QV2_CF_P012},
        {"P212", QV2_CF_P212},

        // Packed Formats
        {"YUY2", QV2_CF_YUY2},
        {"UYVY", QV2_CF_UYVY},
        {"Y210", QV2_CF_Y210},
        {"Y410", QV2_CF_Y410},
        {"Y212", QV2_CF_Y212},
        {"Y412", QV2_CF_Y412}
};

static const unordered_map<string, Qv2APVProfile> ProfileMap = {
        {"422_10", QV2_APV_PROFILE_422_10},
        {"422_12", QV2_APV_PROFILE_422_12},
        {"444_10", QV2_APV_PROFILE_444_10},
        {"444_12", QV2_APV_PROFILE_444_12},
        {"4444_10", QV2_APV_PROFILE_4444_10},
        {"4444_12", QV2_APV_PROFILE_4444_12}
};

static const unordered_map<string, Qv2APVFamily> FamilyMap = {
        {"422_LQ", QV2_APV_FAMILY_422_LQ},
        {"422_SQ", QV2_APV_FAMILY_422_SQ},
        {"422_HQ", QV2_APV_FAMILY_422_HQ},
        {"444_UQ", QV2_APV_FAMILY_444_UQ}
};

static const unordered_map<string, Qv2ColorPrimaries> PrimariesMap = {
        {"bt709", QV2_CP_BT709},
        {"unspecified", QV2_CP_UNSPECIFIED},
        {"uspecified", QV2_CP_UNSPECIFIED},
        {"reserved", QV2_CP_RESERVED},
        {"bt470m", QV2_CP_BT470M},
        {"bt470bg", QV2_CP_BT470BG},
        {"smpte170m", QV2_CP_SMPTE170M},
        {"smpte240m", QV2_CP_SMPTE240M},
        {"film", QV2_CP_FILM},
        {"bt2020", QV2_CP_BT2020},
        {"smpte4280", QV2_CP_SMPTE4280},
        {"smpte4311", QV2_CP_SMPTE4311},
        {"smpte4322", QV2_CP_SMPTE4322}
};

static const unordered_map<string, Qv2ColorTransfer> TransferMap = {
        {"bt709", QV2_CT_BT709},
        {"unspecified", QV2_CT_UNSPECIFIED},
        {"bt470m", QV2_CT_BT470M},
        {"bt470bg", QV2_CT_BT470BG},
        {"smpte170m", QV2_CT_SMPTE170M},
        {"smpte240m", QV2_CT_SMPTE240M},
        {"linear", QV2_CT_LINEAR},
        {"log100", QV2_CT_LOG100},
        {"log316", QV2_CT_LOG316},
        {"iec61966-2-1", QV2_CT_IEC61966_2_1},
        {"bt1361e", QV2_CT_BT1361E},
        {"bt2020-10", QV2_CT_BT2020_10},
        {"bt2020-12", QV2_CT_BT2020_12},
        {"smpte2084", QV2_CT_SMPTE2084},
        {"smpte428", QV2_CT_SMPTE428},
        {"hybrid log-gamma", QV2_CT_HLG_ARIB_STD_B67}
};

static const unordered_map<string, Qv2ColorMatrix> MatrixMap = {
        {"gbr", QV2_CM_GBR},
        {"bt709", QV2_CM_BT709},
        {"unspecified", QV2_CM_UNSPECIFIED},
        {"fcc", QV2_CM_FCC},
        {"bt470bg", QV2_CM_BT470BG},
        {"smpte170m", QV2_CM_SMPTE170M},
        {"smpte240m", QV2_CM_SMPTE240M},
        {"ycgco", QV2_CM_YCGCO},
        {"bt2020nc", QV2_CM_BT2020NC},
        {"bt2020c", QV2_CM_BT2020C},
        {"smpte2085", QV2_CM_SMPTE2085},
        {"chroma-derived-nc", QV2_CM_CHROMA_DERIVED_NC},
        {"chroma-derived-c", QV2_CM_CHROMA_DERIVED_C},
        {"ictcp", QV2_CM_ICTCP}
};

template <typename T>
T getValue(const unordered_map<string, T>& map, const string& key, T defaultValue) {
    auto it = map.find(key);
    return (it != map.end()) ? it->second : defaultValue;
}

#endif // ENCODERVIEWMODEL_H