#ifndef QV2CONSTANTS_H
#define QV2CONSTANTS_H

#pragma once

enum Qv2ColorFormat: int32_t {
    QV2_CF_UNKNOWN                   = 0x0000,
    QV2_CF_YCBCR400                  = 0x080A,
    QV2_CF_YCBCR420                  = 0x080B,
    QV2_CF_YCBCR422                  = 0x080C,
    QV2_CF_YCBCR444                  = 0x080D,
    QV2_CF_YCBCR4444                 = 0x080E,
    QV2_CF_YCBCR400_10LE             = 0x0A0A,
    QV2_CF_YCBCR420_10LE             = 0x0A0B,
    QV2_CF_YCBCR422_10LE             = 0x0A0C,
    QV2_CF_YCBCR444_10LE             = 0x0A0D,
    QV2_CF_YCBCR4444_10LE            = 0x0A0E,
    QV2_CF_YCBCR400_12LE             = 0x0C0A,
    QV2_CF_YCBCR420_12LE             = 0x0C0B,
    QV2_CF_YCBCR422_12LE             = 0x0C0C,
    QV2_CF_YCBCR444_12LE             = 0x0C0D,
    QV2_CF_YCBCR4444_12LE            = 0x0C0E,
    QV2_CF_P210                      = 0x0A14
};

enum Qv2ColorRange: int32_t {
    QV2_CR_UNKNOWN                   = -1,
    QV2_CR_LIMITED                   = 0,
    QV2_CR_FULL                      = 1
};

enum Qv2ColorPrimaries: int32_t {
    QV2_CP_UNKNOWN                   = -1,
    QV2_CP_BT709                     = 1,
    QV2_CP_UNSPECIFIED               = 2,
    QV2_CP_RESERVED                  = 3,
    QV2_CP_BT470M                    = 4,
    QV2_CP_BT470BG                   = 5,
    QV2_CP_SMPTE170M                 = 6,
    QV2_CP_SMPTE240M                 = 7,
    QV2_CP_FILM                      = 8,
    QV2_CP_BT2020                    = 9,
    QV2_CP_SMPTE4280                 = 10,
    QV2_CP_SMPTE4311                 = 11,
    QV2_CP_SMPTE4322                 = 12
};

enum Qv2ColorTransfer: int32_t {
    QV2_CT_UNKNOWN                   = -1,
    QV2_CT_BT709                     = 1,
    QV2_CT_UNSPECIFIED               = 2,
    QV2_CT_BT470M                    = 4,
    QV2_CT_BT470BG                   = 5,
    QV2_CT_SMPTE170M                 = 6,
    QV2_CT_SMPTE240M                 = 7,
    QV2_CT_LINEAR                    = 8,
    QV2_CT_LOG100                    = 9,
    QV2_CT_LOG316                    = 10,
    QV2_CT_IEC61966_2_4              = 11,
    QV2_CT_BT1361E                   = 12,
    QV2_CT_IEC61966_2_1              = 13,
    QV2_CT_BT2020_10                 = 14,
    QV2_CT_BT2020_12                 = 15,
    QV2_CT_SMPTE2084                 = 16,
    QV2_CT_SMPTE428                  = 17,
    QV2_CT_HLG_ARIB_STD_B67          = 18
};

enum Qv2ColorMatrix: int32_t {
    QV2_CM_UNKNOWN                   = -1,
    QV2_CM_GBR                       = 0,
    QV2_CM_BT709                     = 1,
    QV2_CM_UNSPECIFIED               = 2,
    QV2_CM_FCC                       = 4,
    QV2_CM_BT470BG                   = 5,
    QV2_CM_SMPTE170M                 = 6,
    QV2_CM_SMPTE240M                 = 7,
    QV2_CM_YCGCO                     = 8,
    QV2_CM_BT2020NC                  = 9,
    QV2_CM_BT2020C                   = 10,
    QV2_CM_SMPTE2085                 = 11,
    QV2_CM_CHROMA_DERIVED_NC         = 12,
    QV2_CM_CHROMA_DERIVED_C          = 13,
    QV2_CM_ICTCP                     = 14
};


#endif // QV2CONSTANTS_H
