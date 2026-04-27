#ifndef QV2PARAMS_H
#define QV2PARAMS_H

#include <cstdint>
#include <vector>
#include <memory>
#include <string>

/**
 * @brief 32-bit ID Structure for Parameters
 * Bits 31-28: Scope (Global, Input, Output)
 * Bits 27-24: Kind (Setting, Tuning, Info)
 * Bits 23-0:  Index
 */
struct Qv2Param {
    enum Scope : uint32_t {
        GLOBAL = 0, INPUT = 1, OUTPUT = 2
    };
    enum Kind : uint32_t {
        SETTING = 0, TUNING = 1, INFO = 2
    };

    enum Index : uint32_t {
        kIndexVideoSize = 0x01,
        kIndexBitrate = 0x02,
        kIndexFrameRate = 0x03,
        kIndexBitDepth = 0x04,
        kIndexColorFormat = 0x05,
        kIndexProfileLevel = 0x06,
        kIndexQP = 0x09
    };

    uint32_t mId;
    size_t mSize;

    Qv2Param(uint32_t id, size_t size) : mId(id), mSize(size) {}

    virtual ~Qv2Param() = default;

    static constexpr uint32_t
    makeId(Scope
    scope,
    Kind kind, uint32_t
    index) {
        return (static_cast<uint32_t>(scope) << 28) |
               (static_cast<uint32_t>(kind) << 24) |
               (index & 0xFFFFFF);
    }

    template<typename T>
    static T *cast(Qv2Param *param) {
        if (param && param->mId == T::ID) {
            return static_cast<T *>(param);
        }
        return nullptr;
    }
};

/**
 * @brief Video Size (Width x Height)
 */
template<Qv2Param::Scope S>
struct Qv2VideoSizeStreamT : public Qv2Param {
    static constexpr uint32_t
    ID = makeId(S, SETTING, kIndexVideoSize);
    int mWidth;
    int mHeight;

    Qv2VideoSizeStreamT() : Qv2Param(ID, sizeof(Qv2VideoSizeStreamT)),
                            mWidth(0), mHeight(0) {}
};

using Qv2VideoSizeInput = Qv2VideoSizeStreamT<Qv2Param::INPUT>;
using Qv2VideoSizeOutput = Qv2VideoSizeStreamT<Qv2Param::OUTPUT>;

/**
 * @brief Bitrate (bps)
 */
struct Qv2BitrateSetting : public Qv2Param {
    static constexpr uint32_t
    ID = makeId(OUTPUT, SETTING, kIndexBitrate);
    uint32_t mBitrate;

    Qv2BitrateSetting() : Qv2Param(ID, sizeof(Qv2BitrateSetting)),
                          mBitrate(0) {}
};

/**
 * @brief Frame Rate
 */
template<Qv2Param::Scope S>
struct Qv2FrameRateSettingT : public Qv2Param {
    static constexpr uint32_t
    ID = makeId(S, SETTING, kIndexFrameRate);
    float mFps;

    Qv2FrameRateSettingT() : Qv2Param(ID, sizeof(Qv2FrameRateSettingT)),
                             mFps(0.0f) {}
};

using Qv2FrameRateInput = Qv2FrameRateSettingT<Qv2Param::INPUT>;
using Qv2FrameRateOutput = Qv2FrameRateSettingT<Qv2Param::OUTPUT>;

/**
 * @brief Bit Depth (8, 10, 12)
 */
template<Qv2Param::Scope S>
struct Qv2BitDepthSettingT : public Qv2Param {
    static constexpr uint32_t
    ID = makeId(S, SETTING, kIndexBitDepth);
    int mBitDepth;

    Qv2BitDepthSettingT() : Qv2Param(ID, sizeof(Qv2BitDepthSettingT)),
                            mBitDepth(10) {}
};

using Qv2BitDepthInput = Qv2BitDepthSettingT<Qv2Param::INPUT>;
using Qv2BitDepthOutput = Qv2BitDepthSettingT<Qv2Param::OUTPUT>;

/**
 * @brief Color Format (e.g., YUV420, YUV422)
 */
template<Qv2Param::Scope S>
struct Qv2ColorFormatSettingT : public Qv2Param {
    static constexpr uint32_t
    ID = makeId(S, SETTING, kIndexColorFormat);
    int mColorFormat;

    Qv2ColorFormatSettingT() : Qv2Param(ID, sizeof(Qv2ColorFormatSettingT)),
                               mColorFormat(0) {}
};

using Qv2ColorFormatInput = Qv2ColorFormatSettingT<Qv2Param::INPUT>;
using Qv2ColorFormatOutput = Qv2ColorFormatSettingT<Qv2Param::OUTPUT>;

/**
 * @brief Quantization Parameter (QP)
 */
template<Qv2Param::Scope S>
struct Qv2QPSettingT : public Qv2Param {
    static constexpr uint32_t
    ID = makeId(S, SETTING, kIndexQP);
    int mQP;

    Qv2QPSettingT() : Qv2Param(ID, sizeof(Qv2QPSettingT)), mQP(0) {}
};

using Qv2QPInput = Qv2QPSettingT<Qv2Param::INPUT>;

/**
 * @brief Profile and Level (e.g., H.264/AVC Profile and Level)
 */
template<Qv2Param::Scope S>
struct Qv2ProfileLevelT : public Qv2Param {
    static constexpr uint32_t
    ID = makeId(S, SETTING, kIndexProfileLevel);
    int mProfile;
    int mLevel;

    Qv2ProfileLevelT() : Qv2Param(ID, sizeof(Qv2ProfileLevelT)),
                         mProfile(0), mLevel(0) {}
};

using Qv2ProfileLevelInput = Qv2ProfileLevelT<Qv2Param::INPUT>;
using Qv2ProfileLevelOutput = Qv2ProfileLevelT<Qv2Param::OUTPUT>;

#endif // QV2PARAMS_H
