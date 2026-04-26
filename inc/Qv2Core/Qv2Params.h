#ifndef QV2PARAMS_H
#define QV2PARAMS_H

#include <cstdint>
#include <vector>
#include <memory>

/**
 * @brief 32-bit ID Structure for Parameters
 * Bits 31-28: Scope (Global, Input, Output)
 * Bits 27-24: Kind (Setting, Tuning, Info)
 * Bits 23-0:  Index
 */
struct Qv2Param {
    enum Scope : uint32_t { GLOBAL = 0, INPUT = 1, OUTPUT = 2 };
    enum Kind  : uint32_t { SETTING = 0, TUNING = 1, INFO = 2 };

    enum Index : uint32_t {
        kIndexVideoSize   = 0x01,
        kIndexBitrate     = 0x02,
        kIndexFrameRate   = 0x03,
        kIndexBitDepth    = 0x04,
        kIndexColorFormat = 0x05,
        kIndexProfile     = 0x06,
        kIndexLevel       = 0x07,
        kIndexBand        = 0x08,
        kIndexQP          = 0x09,
        kIndexThreads     = 0x0E,
        kIndexPreset      = 0x0F,
        kIndexFamily      = 0x10,
        kIndexMaxAU       = 0x11,
        kIndexSeek        = 0x12
    };

    uint32_t mId;
    size_t mSize;

    Qv2Param(uint32_t id, size_t size) : mId(id), mSize(size) {}
    virtual ~Qv2Param() = default;

    static constexpr uint32_t makeId(Scope scope, Kind kind, uint32_t index) {
        return (static_cast<uint32_t>(scope) << 28) |
               (static_cast<uint32_t>(kind) << 24) |
               (index & 0xFFFFFF);
    }

    template<typename T>
    static T* cast(Qv2Param* param) {
        if (param && param->mId == T::ID) {
            return static_cast<T*>(param);
        }
        return nullptr;
    }
};

/**
 * @brief Video Size (Width x Height)
 */
template<Qv2Param::Scope S>
struct Qv2VideoSizeStreamT : public Qv2Param {
    static constexpr uint32_t ID = makeId(S, SETTING, kIndexVideoSize);
    int mWidth;
    int mHeight;
    Qv2VideoSizeStreamT() : Qv2Param(ID, sizeof(Qv2VideoSizeStreamT)), 
                            mWidth(0), mHeight(0) {}
};

using Qv2VideoSizeInput  = Qv2VideoSizeStreamT<Qv2Param::INPUT>;
using Qv2VideoSizeOutput = Qv2VideoSizeStreamT<Qv2Param::OUTPUT>;

/**
 * @brief Bitrate (bps)
 */
struct Qv2BitrateSetting : public Qv2Param {
    static constexpr uint32_t ID = makeId(OUTPUT, SETTING, kIndexBitrate);
    uint32_t mBitrate; 
    Qv2BitrateSetting() : Qv2Param(ID, sizeof(Qv2BitrateSetting)), 
                          mBitrate(0) {}
};

/**
 * @brief Frame Rate
 */
template<Qv2Param::Scope S>
struct Qv2FrameRateSettingT : public Qv2Param {
    static constexpr uint32_t ID = makeId(S, SETTING, kIndexFrameRate);
    float mFps;
    Qv2FrameRateSettingT() : Qv2Param(ID, sizeof(Qv2FrameRateSettingT)), 
                             mFps(0.0f) {}
};

using Qv2FrameRateInput  = Qv2FrameRateSettingT<Qv2Param::INPUT>;
using Qv2FrameRateOutput = Qv2FrameRateSettingT<Qv2Param::OUTPUT>;

/**
 * @brief Bit Depth (8, 10, 12)
 */
template<Qv2Param::Scope S>
struct Qv2BitDepthSettingT : public Qv2Param {
    static constexpr uint32_t ID = makeId(S, SETTING, kIndexBitDepth);
    int mBitDepth;
    Qv2BitDepthSettingT() : Qv2Param(ID, sizeof(Qv2BitDepthSettingT)), 
                            mBitDepth(8) {}
};

using Qv2BitDepthInput  = Qv2BitDepthSettingT<Qv2Param::INPUT>;
using Qv2BitDepthOutput = Qv2BitDepthSettingT<Qv2Param::OUTPUT>;

/**
 * @brief Color Format (e.g., YUV420, YUV422)
 */
template<Qv2Param::Scope S>
struct Qv2ColorFormatSettingT : public Qv2Param {
    static constexpr uint32_t ID = makeId(S, SETTING, kIndexColorFormat);
    int mColorFormat; 
    Qv2ColorFormatSettingT() : Qv2Param(ID, sizeof(Qv2ColorFormatSettingT)), 
                               mColorFormat(0) {}
};

using Qv2ColorFormatInput  = Qv2ColorFormatSettingT<Qv2Param::INPUT>;
using Qv2ColorFormatOutput = Qv2ColorFormatSettingT<Qv2Param::OUTPUT>;

/**
 * @brief Profile IDC
 */
template<Qv2Param::Scope S>
struct Qv2ProfileSettingT : public Qv2Param {
    static constexpr uint32_t ID = makeId(S, SETTING, kIndexProfile);
    int mProfile;
    Qv2ProfileSettingT() : Qv2Param(ID, sizeof(Qv2ProfileSettingT)), 
                           mProfile(0) {}
};

using Qv2ProfileInput  = Qv2ProfileSettingT<Qv2Param::INPUT>;
using Qv2ProfileOutput = Qv2ProfileSettingT<Qv2Param::OUTPUT>;

/**
 * @brief Level IDC
 */
template<Qv2Param::Scope S>
struct Qv2LevelSettingT : public Qv2Param {
    static constexpr uint32_t ID = makeId(S, SETTING, kIndexLevel);
    int mLevel;
    Qv2LevelSettingT() : Qv2Param(ID, sizeof(Qv2LevelSettingT)), 
                         mLevel(0) {}
};

using Qv2LevelInput  = Qv2LevelSettingT<Qv2Param::INPUT>;
using Qv2LevelOutput = Qv2LevelSettingT<Qv2Param::OUTPUT>;

/**
 * @brief Band IDC
 */
template<Qv2Param::Scope S>
struct Qv2BandSettingT : public Qv2Param {
    static constexpr uint32_t ID = makeId(S, SETTING, kIndexBand);
    int mBand;
    Qv2BandSettingT() : Qv2Param(ID, sizeof(Qv2BandSettingT)), 
                        mBand(0) {}
};

using Qv2BandInput  = Qv2BandSettingT<Qv2Param::INPUT>;
using Qv2BandOutput = Qv2BandSettingT<Qv2Param::OUTPUT>;

/**
 * @brief Quantization Parameter (QP)
 */
template<Qv2Param::Scope S>
struct Qv2QPSettingT : public Qv2Param {
    static constexpr uint32_t ID = makeId(S, SETTING, kIndexQP);
    int mQP;
    Qv2QPSettingT() : Qv2Param(ID, sizeof(Qv2QPSettingT)), mQP(0) {}
};

using Qv2QPInput = Qv2QPSettingT<Qv2Param::INPUT>;


/**
 * @brief Threads Setting
 */
struct Qv2ThreadsSetting : public Qv2Param {
    static constexpr uint32_t ID = makeId(GLOBAL, SETTING, kIndexThreads);
    std::string mThreads;
    Qv2ThreadsSetting() : Qv2Param(ID, sizeof(Qv2ThreadsSetting)), mThreads("auto") {}
};

/**
 * @brief Preset Setting
 */
struct Qv2PresetSetting : public Qv2Param {
    static constexpr uint32_t ID = makeId(GLOBAL, SETTING, kIndexPreset);
    std::string mPreset;
    Qv2PresetSetting() : Qv2Param(ID, sizeof(Qv2PresetSetting)) {}
};

/**
 * @brief Family Setting
 */
struct Qv2FamilySetting : public Qv2Param {
    static constexpr uint32_t ID = makeId(GLOBAL, SETTING, kIndexFamily);
    std::string mFamily;
    Qv2FamilySetting() : Qv2Param(ID, sizeof(Qv2FamilySetting)) {}
};

/**
 * @brief Max Access Units
 */
struct Qv2MaxAUSetting : public Qv2Param {
    static constexpr uint32_t ID = makeId(GLOBAL, SETTING, kIndexMaxAU);
    int mMaxAU;
    Qv2MaxAUSetting() : Qv2Param(ID, sizeof(Qv2MaxAUSetting)), mMaxAU(0) {}
};

/**
 * @brief Seek Setting
 */
struct Qv2SeekSetting : public Qv2Param {
    static constexpr uint32_t ID = makeId(GLOBAL, SETTING, kIndexSeek);
    int mSeek;
    Qv2SeekSetting() : Qv2Param(ID, sizeof(Qv2SeekSetting)), mSeek(0) {}
};




#endif // QV2PARAMS_H
