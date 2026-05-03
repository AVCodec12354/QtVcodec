#ifndef QV2CONSTANTS_H
#define QV2CONSTANTS_H

#pragma once

enum Qv2APVProfile : int32_t {
    QV2_APV_PROFILE_422_10 = 33,
    QV2_APV_PROFILE_422_12 = 44,
    QV2_APV_PROFILE_444_10 = 55,
    QV2_APV_PROFILE_444_12 = 66,
    QV2_APV_PROFILE_4444_10 = 77,
    QV2_APV_PROFILE_4444_12 = 88,
    QV2_APV_PROFILE_400_10 = 99,
};

enum Qv2APVLevel : int32_t {
    // Band 0
    QV2_APV_LEVEL_1_BAND_0           = 0x0000,
    QV2_APV_LEVEL_1_1_BAND_0         = 0x0001,
    QV2_APV_LEVEL_2_BAND_0           = 0x0002,
    QV2_APV_LEVEL_2_1_BAND_0         = 0x0003,
    QV2_APV_LEVEL_3_BAND_0           = 0x0004,
    QV2_APV_LEVEL_3_1_BAND_0         = 0x0005,
    QV2_APV_LEVEL_4_BAND_0           = 0x0006,
    QV2_APV_LEVEL_4_1_BAND_0         = 0x0007,
    QV2_APV_LEVEL_5_BAND_0           = 0x0008,
    QV2_APV_LEVEL_5_1_BAND_0         = 0x0009,
    QV2_APV_LEVEL_6_BAND_0           = 0x000A,
    QV2_APV_LEVEL_6_1_BAND_0         = 0x000B,
    QV2_APV_LEVEL_7_BAND_0           = 0x000C,
    QV2_APV_LEVEL_7_1_BAND_0         = 0x000D,

    // Band 1
    QV2_APV_LEVEL_1_BAND_1           = 0x0100,
    QV2_APV_LEVEL_1_1_BAND_1         = 0x0101,
    QV2_APV_LEVEL_2_BAND_1           = 0x0102,
    QV2_APV_LEVEL_2_1_BAND_1         = 0x0103,
    QV2_APV_LEVEL_3_BAND_1           = 0x0104,
    QV2_APV_LEVEL_3_1_BAND_1         = 0x0105,
    QV2_APV_LEVEL_4_BAND_1           = 0x0106,
    QV2_APV_LEVEL_4_1_BAND_1         = 0x0107,
    QV2_APV_LEVEL_5_BAND_1           = 0x0108,
    QV2_APV_LEVEL_5_1_BAND_1         = 0x0109,
    QV2_APV_LEVEL_6_BAND_1           = 0x010A,
    QV2_APV_LEVEL_6_1_BAND_1         = 0x010B,
    QV2_APV_LEVEL_7_BAND_1           = 0x010C,
    QV2_APV_LEVEL_7_1_BAND_1         = 0x010D,

    // Band 2
    QV2_APV_LEVEL_1_BAND_2           = 0x0200,
    QV2_APV_LEVEL_1_1_BAND_2         = 0x0201,
    QV2_APV_LEVEL_2_BAND_2           = 0x0202,
    QV2_APV_LEVEL_2_1_BAND_2         = 0x0203,
    QV2_APV_LEVEL_3_BAND_2           = 0x0204,
    QV2_APV_LEVEL_3_1_BAND_2         = 0x0205,
    QV2_APV_LEVEL_4_BAND_2           = 0x0206,
    QV2_APV_LEVEL_4_1_BAND_2         = 0x0207,
    QV2_APV_LEVEL_5_BAND_2           = 0x0208,
    QV2_APV_LEVEL_5_1_BAND_2         = 0x0209,
    QV2_APV_LEVEL_6_BAND_2           = 0x020A,
    QV2_APV_LEVEL_6_1_BAND_2         = 0x020B,
    QV2_APV_LEVEL_7_BAND_2           = 0x020C,
    QV2_APV_LEVEL_7_1_BAND_2         = 0x020D,

    // Band 3
    QV2_APV_LEVEL_1_BAND_3           = 0x0300,
    QV2_APV_LEVEL_1_1_BAND_3         = 0x0301,
    QV2_APV_LEVEL_2_BAND_3           = 0x0302,
    QV2_APV_LEVEL_2_1_BAND_3         = 0x0303,
    QV2_APV_LEVEL_3_BAND_3           = 0x0304,
    QV2_APV_LEVEL_3_1_BAND_3         = 0x0305,
    QV2_APV_LEVEL_4_BAND_3           = 0x0306,
    QV2_APV_LEVEL_4_1_BAND_3         = 0x0307,
    QV2_APV_LEVEL_5_BAND_3           = 0x0308,
    QV2_APV_LEVEL_5_1_BAND_3         = 0x0309,
    QV2_APV_LEVEL_6_BAND_3           = 0x030A,
    QV2_APV_LEVEL_6_1_BAND_3         = 0x030B,
    QV2_APV_LEVEL_7_BAND_3           = 0x030C,
    QV2_APV_LEVEL_7_1_BAND_3         = 0x030D,
};

enum Qv2APVFamily : int32_t {
    QV2_APV_FAMILY_422_LQ = 1,
    QV2_APV_FAMILY_422_SQ = 2,
    QV2_APV_FAMILY_422_HQ = 3,
    QV2_APV_FAMILY_444_UQ = 4,
};
enum Qv2APVPreset : int32_t {
    QV2_PRESET_FASTEST = 0,
    QV2_PRESET_FAST = 1,
    QV2_PRESET_MEDIUM = 2,
    QV2_PRESET_SLOW = 3,
    QV2_PRESET_PLACEBO = 4,
    QV2_PRESET_DEFAULT = QV2_PRESET_MEDIUM
};

enum Qv2ColorFormat : int32_t {
    QV2_CF_UNKNOWN = 0x0000,

    // Planar Formats (Y, U, V separate planes)
    QV2_CF_YCBCR400 = 0x080A,
    QV2_CF_YCBCR420 = 0x080B,
    QV2_CF_YCBCR422 = 0x080C,
    QV2_CF_YCBCR444 = 0x080D,
    QV2_CF_YCBCR4444 = 0x080E,

    QV2_CF_YCBCR400_10LE = 0x0A0A,
    QV2_CF_YCBCR420_10LE = 0x0A0B,
    QV2_CF_YCBCR422_10LE = 0x0A0C,
    QV2_CF_YCBCR444_10LE = 0x0A0D,
    QV2_CF_YCBCR4444_10LE = 0x0A0E,

    QV2_CF_YCBCR400_12LE = 0x0C0A,
    QV2_CF_YCBCR420_12LE = 0x0C0B,
    QV2_CF_YCBCR422_12LE = 0x0C0C,
    QV2_CF_YCBCR444_12LE = 0x0C0D,
    QV2_CF_YCBCR4444_12LE = 0x0C0E,

    // Semi-Planar Formats (Y plane + Interleaved UV plane)
    QV2_CF_NV12 = 0x0814, // 8-bit 4:2:0 (Y + UV)
    QV2_CF_NV21 = 0x0815, // 8-bit 4:2:0 (Y + VU)
    QV2_CF_NV16 = 0x0816, // 8-bit 4:2:2 (Y + UV)
    QV2_CF_P010 = 0x0A14, // 10-bit 4:2:0 (Y + UV)
    QV2_CF_P210 = 0x0A15, // 10-bit 4:2:2 (Y + UV)
    QV2_CF_P012 = 0x0C14, // 12-bit 4:2:0 (Y + UV)
    QV2_CF_P212 = 0x0C15, // 12-bit 4:2:2 (Y + UV)

    // Packed Formats (Y, U, V interleaved in a single plane)
    QV2_CF_YUY2 = 0x0820, // 8-bit 4:2:2 (YUYV)
    QV2_CF_UYVY = 0x0821, // 8-bit 4:2:2 (UYVY)
    QV2_CF_Y210 = 0x0A20, // 10-bit 4:2:2 Packed
    QV2_CF_Y410 = 0x0A21, // 10-bit 4:4:4 Packed
    QV2_CF_Y212 = 0x0C20, // 12-bit 4:2:2 Packed
    QV2_CF_Y412 = 0x0C21  // 12-bit 4:4:4 Packed
};

#define QV2_SUBSAMPLING_400  0
#define QV2_SUBSAMPLING_420  1
#define QV2_SUBSAMPLING_422  2
#define QV2_SUBSAMPLING_444  3

#define CF_IS_SEMI_PLANAR(fmt) ( \
    (fmt) == QV2_CF_NV12 ||        \
    (fmt) == QV2_CF_NV21 ||        \
    (fmt) == QV2_CF_P010 ||        \
    (fmt) == QV2_CF_P012 ||        \
    (fmt) == QV2_CF_NV16 ||        \
    (fmt) == QV2_CF_P210 ||        \
    (fmt) == QV2_CF_P212           \
)

#define CF_IS_PACKED(fmt) ( \
    ((fmt) == QV2_CF_YUY2 ||         \
    (fmt) == QV2_CF_Y210  ||         \
    (fmt) == QV2_CF_Y212)  ? 1 :     \
                                     \
    ((fmt) == QV2_CF_UYVY) ? 2 :     \
                                     \
    ((fmt) == QV2_CF_Y410 ||         \
    (fmt) == QV2_CF_Y412)  ? 3 : 0   \
)

#define QV2_GET_BIT_DEPTH(fmt) ( \
    ((fmt) >= 0x0C00) ? 12 : \
    ((fmt) >= 0x0A00) ? 10 : 8 \
)

#define QV2_GET_SUBSAMPLING(fmt) ( \
    /* 400 */                      \
    ((fmt) == QV2_CF_YCBCR400 ||   \
    (fmt) == QV2_CF_YCBCR400_10LE || \
    (fmt) == QV2_CF_YCBCR400_12LE) ? QV2_SUBSAMPLING_400 : \
    /* 420 */ \
    ((fmt) == QV2_CF_YCBCR420 || \
     (fmt) == QV2_CF_YCBCR420_10LE || \
     (fmt) == QV2_CF_YCBCR420_12LE || \
     (fmt) == QV2_CF_NV12 || \
     (fmt) == QV2_CF_NV21 || \
     (fmt) == QV2_CF_P010 || \
     (fmt) == QV2_CF_P012) ? QV2_SUBSAMPLING_420 : \
    \
    /* 422 */ \
    ((fmt) == QV2_CF_YCBCR422 || \
     (fmt) == QV2_CF_YCBCR422_10LE || \
     (fmt) == QV2_CF_YCBCR422_12LE || \
     (fmt) == QV2_CF_NV16 || \
     (fmt) == QV2_CF_P210 || \
     (fmt) == QV2_CF_P212 || \
     (fmt) == QV2_CF_YUY2 || \
     (fmt) == QV2_CF_UYVY || \
     (fmt) == QV2_CF_Y210 || \
     (fmt) == QV2_CF_Y212) ? QV2_SUBSAMPLING_422 : \
    \
    /* default 444 */ \
    QV2_SUBSAMPLING_444 \
)

#define QV2_GET_PLANE_SIZE(fmt, plane, width, height, outW, outH) \
do { \
    outW = width; \
    outH = height; \
    /* 1. MONOCHROME (4:0:0) - Chỉ có Plane 0 */ \
    if (QV2_GET_SUBSAMPLING(fmt) == QV2_SUBSAMPLING_400) { \
        if (plane == 0) { outW = width; outH = height; } \
    } \
    /* 2. PACKED FORMATS (YUY2, UYVY, Y210, Y410...) - Luôn chỉ có Plane 0 */ \
    else if (CF_IS_PACKED(fmt)) { \
        if (plane == 0) { outW = (width + 1) / 2; outH = height; } \
    } \
    /* 3. SEMI-PLANAR (NV12, P010, NV16, P210...) - Có 2 Planes */ \
    else if (CF_IS_SEMI_PLANAR(fmt)) { \
        if (plane == 0) { \
            outW = width; outH = height; \
        } else if (plane == 1) { \
            outW = (width + 1) / 2; \
            if (QV2_GET_SUBSAMPLING(fmt) == QV2_SUBSAMPLING_420) \
                outH = (height + 1) / 2; \
            else \
                outH = height; \
        } \
    } \
    /* 4. PLANAR FORMATS (YUV separate) */ \
    else { \
        if (plane == 0) { \
            outW = width; outH = height; \
        } else if (plane == 1 || plane == 2) { \
            int sub = QV2_GET_SUBSAMPLING(fmt); \
            if (sub == QV2_SUBSAMPLING_420) { \
                outW = (width + 1) / 2; outH = (height + 1) / 2; \
            } else if (sub == QV2_SUBSAMPLING_422) { \
                outW = (width + 1) / 2; outH = height; \
            } else { \
                outW = width; outH = height; \
            } \
        } else if (plane == 3) { \
            /* Plane Alpha cho 4:4:4:4 */ \
            if ((fmt) == QV2_CF_YCBCR4444 || \
            (fmt) == QV2_CF_YCBCR4444_10LE || \
            (fmt) == QV2_CF_YCBCR4444_12LE) { \
                outW = width; outH = height; \
            } \
        } \
    } \
} while(0)

enum Qv2ColorRange : int32_t {
    QV2_CR_UNKNOWN = -1,
    QV2_CR_LIMITED = 0,
    QV2_CR_FULL = 1
};

enum Qv2ColorPrimaries : int32_t {
    QV2_CP_UNKNOWN = -1,
    QV2_CP_BT709 = 1,
    QV2_CP_UNSPECIFIED = 2,
    QV2_CP_RESERVED = 3,
    QV2_CP_BT470M = 4,
    QV2_CP_BT470BG = 5,
    QV2_CP_SMPTE170M = 6,
    QV2_CP_SMPTE240M = 7,
    QV2_CP_FILM = 8,
    QV2_CP_BT2020 = 9,
    QV2_CP_SMPTE4280 = 10,
    QV2_CP_SMPTE4311 = 11,
    QV2_CP_SMPTE4322 = 12
};

enum Qv2ColorTransfer : int32_t {
    QV2_CT_UNKNOWN = -1,
    QV2_CT_BT709 = 1,
    QV2_CT_UNSPECIFIED = 2,
    QV2_CT_BT470M = 4,
    QV2_CT_BT470BG = 5,
    QV2_CT_SMPTE170M = 6,
    QV2_CT_SMPTE240M = 7,
    QV2_CT_LINEAR = 8,
    QV2_CT_LOG100 = 9,
    QV2_CT_LOG316 = 10,
    QV2_CT_IEC61966_2_4 = 11,
    QV2_CT_BT1361E = 12,
    QV2_CT_IEC61966_2_1 = 13,
    QV2_CT_BT2020_10 = 14,
    QV2_CT_BT2020_12 = 15,
    QV2_CT_SMPTE2084 = 16,
    QV2_CT_SMPTE428 = 17,
    QV2_CT_HLG_ARIB_STD_B67 = 18
};

enum Qv2ColorMatrix : int32_t {
    QV2_CM_UNKNOWN = -1,
    QV2_CM_GBR = 0,
    QV2_CM_BT709 = 1,
    QV2_CM_UNSPECIFIED = 2,
    QV2_CM_FCC = 4,
    QV2_CM_BT470BG = 5,
    QV2_CM_SMPTE170M = 6,
    QV2_CM_SMPTE240M = 7,
    QV2_CM_YCGCO = 8,
    QV2_CM_BT2020NC = 9,
    QV2_CM_BT2020C = 10,
    QV2_CM_SMPTE2085 = 11,
    QV2_CM_CHROMA_DERIVED_NC = 12,
    QV2_CM_CHROMA_DERIVED_C = 13,
    QV2_CM_ICTCP = 14
};


#endif // QV2CONSTANTS_H
