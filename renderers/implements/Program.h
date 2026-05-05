#pragma once

#include <string>
#include <vector>

inline const char *vertexShader = R"(
    attribute vec4 vertexIn;
    attribute vec2 textureIn;
    varying vec2 textureOut;
    void main(void) {
        gl_Position = vertexIn;
        textureOut = textureIn;
    }
)";

static inline void generateFragmentShader(std::string &shaderCode, int numOfPlane) {
    shaderCode = R"(
        varying vec2 textureOut;

        uniform int isLimitedRange;
        // Packed value
        // 0: No
        // 1: YUY2/Y210/Y212 (4:2:2)
        // 2: UYVY (4:2:2)
        // 3: Y410/Y412 (4:4:4)
        uniform int isPacked;
        uniform int swapUV;
        uniform float bitScale;
        uniform mat3 cs_matrix;

        // Các hằng số cho Limited Range (BT.601/709/2020 limited)
        const float yOffset = 16.0 / 255.0;
        const float yRange  = 219.0 / 255.0;
        const float uvOffset = 128.0 / 255.0;
        const float uvRange  = 224.0 / 255.0;

        vec3 yuvToRgb(float y, float u, float v) {
            // Scale cho 10-bit/12-bit nếu cần (bitScale = 1.0 cho 8-bit)
            y *= bitScale;
            u *= bitScale;
            v *= bitScale;

            float Y, U, V;
            if (isLimitedRange == 1) {
                Y = (y - yOffset) / yRange;
                U = (u - uvOffset) / uvRange;
                V = (v - uvOffset) / uvRange;
            } else {
                Y = y;
                U = u - 0.5;
                V = v - 0.5;
            }
            return clamp(cs_matrix * vec3(Y, U, V), 0.0, 1.0);
        }
    )";

    if (numOfPlane == 3) {
        // Planar: Y, U, V tách biệt (YUV420P, YUV422P, YUV444P)
        shaderCode += R"(
            uniform sampler2D tex0; // Y
            uniform sampler2D tex1; // U
            uniform sampler2D tex2; // V

            void main(void) {
                float y = texture2D(tex0, textureOut).r;
                float u = texture2D(tex1, textureOut).r;
                float v = texture2D(tex2, textureOut).r;
                gl_FragColor = vec4(yuvToRgb(y, u, v), 1.0);
            }
        )";
    } else if (numOfPlane == 2) {
        // Semi-Planar: Y và UV/VU gộp (NV12, NV21, P010, P210)
        shaderCode += R"(
            uniform sampler2D tex0; // Y
            uniform sampler2D tex1; // UV | VU

            void main(void) {
                float y = texture2D(tex0, textureOut).r;
                // R=U, G=V (hoặc ngược lại)
                vec2 uv = texture2D(tex1, textureOut).rg;
                float u = (swapUV == 1) ? uv.y : uv.x;
                float v = (swapUV == 1) ? uv.x : uv.y;
                gl_FragColor = vec4(yuvToRgb(y, u, v), 1.0);
            }
        )";
    } else {
        shaderCode += R"(
            uniform sampler2D tex0;
            uniform vec2 texSize;

            void main(void) {
                if (isPacked == 1 || isPacked == 2) {
                    // --- PACKED 4:2:2 (YUY2, UYVY, Y210, Y212) ---
                    vec4 col = texture2D(tex0, textureOut);
                    float xPixel = textureOut.x * texSize.x;
                    // Nếu là pixel lẻ (isOdd = true), lấy kênh B (Y1)
                    // Nếu là pixel chẵn (isOdd = false), lấy kênh R (Y0)
                    bool isOdd = mod(floor(xPixel), 2.0) > 0.5;

                    float y, u, v;
                    if (isPacked == 1) { // YUY2 / Y210 / Y212 (Y0 U Y1 V)
                        y = isOdd ? col.b : col.r;
                        u = col.g;
                        v = col.a;
                    } else { // UYVY (U Y0 V Y1)
                        y = isOdd ? col.a : col.g;
                        u = col.r;
                        v = col.b;
                    }
                    gl_FragColor = vec4(yuvToRgb(y, u, v), 1.0);

                } else if (isPacked == 3) {
                    // --- PACKED 4:4:4 (Y410, Y412) ---
                    vec4 col = texture2D(tex0, textureOut);
                    gl_FragColor = vec4(yuvToRgb(col.g, col.r, col.b), 1.0);
                } else {
                    // --- GRAYSCALE (Y400) ---
                    float y = texture2D(tex0, textureOut).r;
                    gl_FragColor = vec4(vec3(y * bitScale), 1.0);
                }
            }
        )";
    }
}

// =========================
// Color matrices
// =========================

// BT.601 (SDTV) - Thường dùng cho video độ phân giải thấp
static const float bt601[] = {
        1.164f,  0.000f,  1.596f,
        1.164f, -0.392f, -0.813f,
        1.164f,  2.017f,  0.000f
};

// BT.709 (HDTV) - Chuẩn phổ biến nhất cho video HD/1080p
static const float bt709[] = {
        1.164f,  0.000f,  1.793f,
        1.164f, -0.213f, -0.533f,
        1.164f,  2.112f,  0.000f
};

// BT.2020 (UHDTV) - Dùng cho video 4K/8K và HDR
static const float bt2020[] = {
        1.164f,  0.000f,  1.678f,
        1.164f, -0.187f, -0.650f,
        1.164f,  2.142f,  0.000f
};