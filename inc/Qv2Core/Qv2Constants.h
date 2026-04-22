#ifndef QV2CONSTANTS_H
#define QV2CONSTANTS_H

#pragma once

enum Qv2ColorFormat: int32_t {
    QV2FormatYUV420Flexible          = 0x7F420888,
    QV2FormatYUV420PackedPlanar      = 20,
    QV2FormatYUV420PackedSemiPlanar  = 39,
    QV2FormatYUV420Planar            = 19,
    QV2FormatYUV420SemiPlanar        = 21,
    QV2FormatYUV422Flexible          = 0x7F422888,
    QV2FormatYUV422PackedPlanar      = 23,
    QV2FormatYUV422PackedSemiPlanar  = 40,
    QV2FormatYUV422Planar            = 22,
    QV2FormatYUV422SemiPlanar        = 24,
    QV2FormatYUV444Flexible          = 0x7F444888,
    QV2FormatYUV444Interleaved       = 29,
    QV2FormatYUVP010                 = 54,
    QV2FormatYUVP210                 = 60,
    QV2QCOM_FormatYUV420SemiPlanar   = 0x7fa30c00,
    QV2TI_FormatYUV420PackedSemiPlanar = 0x7f000100,
};

#endif // QV2CONSTANTS_H
