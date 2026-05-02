#include "VideoRenderer.h"

#include <Program.h>

#define MAX_LOG_LENGTH 512

VideoRenderer::VideoRenderer() {
    std::cout << __FUNCTION__  << std::endl;
    isInitialized = false;
}

VideoRenderer::~VideoRenderer() {
    std::cout << __FUNCTION__  << std::endl;
    isInitialized = false;
    glDeleteTextures(MAX_NUM_PLANES, mTextures);
    glDeleteProgram(shaderProgram);
}

void VideoRenderer::createShader(GLuint &shader, GLuint type, const char* shaderCode) {
    std::cout << __FUNCTION__ << std::endl;
    int success;
    char infoLog[MAX_LOG_LENGTH];
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderCode, nullptr);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, MAX_LOG_LENGTH, nullptr, infoLog);
        std::cout << "Fragment shader error: " << infoLog << std::endl;
    }
}

void VideoRenderer::initOpenGL(int numOfPlane) {
    std::cout << __FUNCTION__ << std::endl;
    initializeOpenGLFunctions();
    createShader(vShader, GL_VERTEX_SHADER, vertexShader);
    std::string fragmentShader;
    generateFragmentShader(fragmentShader, numOfPlane);
    std::cout << "Shader code:\n" << fragmentShader << std::endl;
    createShader(fShader, GL_FRAGMENT_SHADER, fragmentShader.c_str());

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vShader);
    glAttachShader(shaderProgram, fShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[MAX_LOG_LENGTH];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, MAX_LOG_LENGTH, nullptr, infoLog);
        std::cout << "Program linking error: " << infoLog << std::endl;
    }
    glDeleteShader(vShader);
    glDeleteShader(fShader);

    glGenTextures(numOfPlane, mTextures);
    for(int i = 0; i < numOfPlane; i++) {
        glBindTexture(GL_TEXTURE_2D, mTextures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    isInitialized = true;
}

void VideoRenderer::renderFrame(std::shared_ptr<Qv2Buffer> buffer) {
    if (!buffer || buffer->graphicBlocks().empty()) return;

    auto block = buffer->graphicBlocks()[0];
    int numPlanes = block->numPlanes();
    int width     = block->width();
    int height    = block->height();
    int bitDepth  = block->bitDepth();
    int format    = block->format();

    if (!isInitialized) {
        initOpenGL(numPlanes);
        updateSurfaceSize(width, height);
    }

    glUseProgram(shaderProgram);

    // 1. Upload Planes
    std::vector<std::string> names = getSamplerNames(numPlanes);
    for (int i = 0; i < numPlanes; ++i) {
        uint8_t* addr = block->addr(i);
        int stride    = block->stride(i);

        int pWidth = width;
        int pHeight = height;

        // Logic tính kích thước Plane chuẩn xác hơn
        if (i > 0) {
            // Hầu hết format YUV là 4:2:0 (NV12, P010, I420) -> Chia cả 2
            pWidth = width / 2;
            pHeight = height / 2;

            // Nếu là 4:2:2 (NV16, P210) -> Chỉ chia Width
            if (format == 0x02 /* Thay bằng enum 422 của bạn */) {
                pHeight = height;
            }
            // Nếu là 4:4:4 -> Không chia gì cả
        }

        GLenum internalFormat, formatGL;
        GLenum type = (bitDepth <= 8) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT;

        if (numPlanes == 2) {
            internalFormat = (i == 0) ? (bitDepth <= 8 ? GL_R8 : GL_R16) : (bitDepth <= 8 ? GL_RG8 : GL_RG16);
            formatGL = (i == 0) ? GL_RED : GL_RG;
        } else {
            internalFormat = (bitDepth <= 8) ? GL_R8 : GL_R16;
            formatGL = GL_RED;
        }

        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, mTextures[i]);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        int bytesPerPixel = (bitDepth <= 8) ? 1 : 2;
        glPixelStorei(GL_UNPACK_ROW_LENGTH, stride / bytesPerPixel);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, pWidth, pHeight, 0, formatGL, type, addr);
        glUniform1i(glGetUniformLocation(shaderProgram, names[i].c_str()), i);
    }
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    // 2. BitScale (🔥 SỬA LỖI ĐEN MÀN HÌNH)
    // Nếu 10-bit và dùng GL_UNSIGNED_SHORT, giá trị 1023 sẽ là 1023/65535 = 0.015 (Rất tối)
    // Chúng ta cần nhân với 64.0 để đưa nó về dải 0.0 - 1.0
    float bitScale = 1.0f;
    if (bitDepth == 10) bitScale = 64.0f;
    else if (bitDepth == 12) bitScale = 16.0f;
    glUniform1f(glGetUniformLocation(shaderProgram, "bitScale"), bitScale);

    // 3. Color Matrix (Dùng GL_FALSE và ma trận chuẩn)
    // Tôi sẽ cung cấp ma trận BT.709 chuẩn Row-Major
    static const float bt709_matrix[] = {
            1.1644f,  0.0000f,  1.7927f,
            1.1644f, -0.2132f, -0.5329f,
            1.1644f,  2.1124f,  0.0000f
    };
    glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "cs_matrix"), 1, GL_TRUE, bt709_matrix);

    // 4. Uniforms & Flags
    // Đưa các giá trị offset về 0.0 nếu là Full Range để test trước
    glUniform1f(glGetUniformLocation(shaderProgram, "yOffset"),  16.0f/255.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "yRange"),   219.0f/255.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "uvOffset"), 128.0f/255.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "uvRange"),  224.0f/255.0f);

    glUniform1i(glGetUniformLocation(shaderProgram, "isLimitedRange"), 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "isNV21"), (format == 0x15));

    bool isYV12 = (numPlanes == 3 && block->addr(1) > block->addr(2));
    glUniform1i(glGetUniformLocation(shaderProgram, "isYV12"), isYV12);

    draw();
}

void VideoRenderer::draw() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLint vertexInLoc = glGetAttribLocation(shaderProgram, "vertexIn");
    glEnableVertexAttribArray(vertexInLoc);
    glVertexAttribPointer(vertexInLoc, 2, GL_FLOAT, GL_FALSE, 0, vertices);

    GLint textureInLoc = glGetAttribLocation(shaderProgram, "textureIn");
    glEnableVertexAttribArray(textureInLoc);
    glVertexAttribPointer(textureInLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(vertexInLoc);
    glDisableVertexAttribArray(textureInLoc);
}

