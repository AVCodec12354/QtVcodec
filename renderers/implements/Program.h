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

        uniform mat3 cs_matrix;
        uniform int isLimitedRange;
        uniform int isNV21;

        uniform float yOffset;
        uniform float yRange;
        uniform float uvOffset;
        uniform float uvRange;
        uniform float bitScale;

        vec3 yuvToRgb(float y, float u, float v) {
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

            return cs_matrix * vec3(Y, U, V);
        }
    )";

    if (numOfPlane == 2) {
        shaderCode += R"(
            uniform sampler2D tex_y;
            uniform sampler2D tex_uv;

            void main(void) {
                float y = texture2D(tex_y, textureOut).r;
                vec2 uv = texture2D(tex_uv, textureOut).rg;

                float u = uv.x;
                float v = uv.y;

                if (isNV21 == 1) {
                    float tmp = u;
                    u = v;
                    v = tmp;
                }

                gl_FragColor = vec4(clamp(yuvToRgb(y, u, v), 0.0, 1.0), 1.0);
            }
        )";
    } else if (numOfPlane == 3) {
        shaderCode += R"(
            uniform sampler2D tex_y;
            uniform sampler2D tex_u;
            uniform sampler2D tex_v;

            void main(void) {
                float y = texture2D(tex_y, textureOut).r;
                float u = texture2D(tex_u, textureOut).r;
                float v = texture2D(tex_v, textureOut).r;

                gl_FragColor = vec4(clamp(yuvToRgb(y, u, v), 0.0, 1.0), 1.0);
            }
        )";
    } else {
        shaderCode += R"(
            uniform sampler2D tex_y;
            void main(void) {
                gl_FragColor = texture2D(tex_y, textureOut);
            }
        )";
    }
}

static inline std::vector<std::string> getSamplerNames(int numOfPlane) {
    switch (numOfPlane) {
        case 2: return {"tex_y", "tex_uv"};
        case 3: return {"tex_y", "tex_u", "tex_v"};
        default: return {"tex_y"};
    }
}

// =========================
// Color matrices
// =========================

struct bt601 {
    const float matrix[9] = {
            1.0f,  0.0f,       1.13983f,
            1.0f, -0.39465f,  -0.58060f,
            1.0f,  2.03211f,   0.0f
    };
};

struct bt709 {
    const float matrix[9] = {
            1.0f,  0.0f,       1.5748f,
            1.0f, -0.187324f, -0.468124f,
            1.0f,  1.8556f,    0.0f
    };
};

struct bt2020 {
    const float matrix[9] = {
            1.0f,  0.0f,       1.4746f,
            1.0f, -0.164553f, -0.571353f,
            1.0f,  1.8814f,    0.0f
    };
};