#ifndef QV2METADATA_H
#define QV2METADATA_H

/**
 * HDR Static Metadata Info.
 */
struct Qv2ColorXyStruct {
    float x; ///< x color coordinate in xyY space [0-1]
    float y; ///< y color coordinate in xyY space [0-1]
};

struct Qv2MasteringDisplayColorVolumeStruct {
    Qv2ColorXyStruct red;    ///< coordinates of red display primary
    Qv2ColorXyStruct green;  ///< coordinates of green display primary
    Qv2ColorXyStruct blue;   ///< coordinates of blue display primary
    Qv2ColorXyStruct white;  ///< coordinates of white point

    float maxLuminance;  ///< max display mastering luminance in cd/m^2
    float minLuminance;  ///< min display mastering luminance in cd/m^2
};

struct Qv2HdrStaticMetadata {
    Qv2MasteringDisplayColorVolumeStruct mastering;
    // content descriptors
    float maxCll;  ///< max content light level (pixel luminance) in cd/m^2
    float maxFall; ///< max frame average light level (frame luminance) in cd/m^2
};

/**
 * color Aspect Info.
 */
struct Qv2ColorAspect {
    int32_t primaries;   ///< Color primaries (Qv2ColorPrimaries)
    int32_t transfer;    ///< Transfer characteristics (Qv2ColorTransfer)
    int32_t matrix;      ///< Color matrix coefficients (Qv2ColorMatrix)
    int32_t range;       ///< Color range (Qv2ColorRange)
};

#endif // QV2METADATA_H