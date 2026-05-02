#include "VideoRenderer.h"

#include <Program.h>

#define MAX_LOG_LENGTH 512

VideoRenderer::VideoRenderer() {
    std::cout << __FUNCTION__  << std::endl;
    isInitialized = false;
}

VideoRenderer::~VideoRenderer() {
    std::cout << __FUNCTION__  << std::endl;
    if (QOpenGLContext::currentContext() && isInitialized) {
        mWidget->makeCurrent();
        glDeleteTextures(MAX_NUM_PLANES, mTextures);
        glDeleteProgram(shaderProgram);
        mWidget->doneCurrent();
    }
    isInitialized = false;
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
    int format    = block->format();
    int width     = block->width();
    int height    = block->height();
    int bitDepth  = block->bitDepth();
    int numPlanes = block->numPlanes();

    if (!isInitialized) {
        initOpenGL(numPlanes);
        updateSurfaceSize(width, height);
    }

    glUseProgram(shaderProgram);
    for (int i = 0; i < numPlanes; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, mTextures[i]);

        int pW, pH;
        QV2_GET_PLANE_SIZE(format, i, width, height, pW, pH);
        GLenum internalFormat, glFormat;
        GLenum type = (bitDepth <= 8) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT;

        if (CF_IS_PACKED(format)) {
            // Packed format (YUY2, Y210...)
            internalFormat = (bitDepth <= 8) ? GL_RGBA8 : GL_RGBA16;
            glFormat = GL_RGBA;
            pW = width / 2;
        } else if (CF_IS_SEMI_PLANAR(format) && i == 1) {
            // Semi-planar (NV12, P010...)
            internalFormat = (bitDepth <= 8) ? GL_RG8 : GL_RG16;
            glFormat = GL_RG;
            pW = width / 2;
        } else {
            // Planar
            internalFormat = (bitDepth <= 8) ? GL_R8 : GL_R16;
            glFormat = GL_RED;
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        int bytesPerComponent = (bitDepth <= 8) ? 1 : 2;
        int componentsPerPixel = (glFormat == GL_RG) ? 2 : (glFormat == GL_RGBA ? 4 : 1);
        int bpp = bytesPerComponent * componentsPerPixel;
        if (block->stride(i) % bpp != 0) {
            std::cerr << "Warning: Stride is not a multiple of pixel size!" << std::endl;
        }
        glPixelStorei(GL_UNPACK_ROW_LENGTH, block->stride(i) / bpp);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, pW, pH, 0, glFormat, type, block->addr(i));
        std::string samplerName = "tex" + std::to_string(i);
        glUniform1i(glGetUniformLocation(shaderProgram, samplerName.c_str()), i);
    }
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glUniform1i(glGetUniformLocation(shaderProgram, "isLimitedRange"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "isPacked"), CF_IS_PACKED(format));
    glUniform1i(glGetUniformLocation(shaderProgram, "isSemiPlanar"), CF_IS_SEMI_PLANAR(format));
    glUniform1i(glGetUniformLocation(shaderProgram, "swapUV"), (format == QV2_CF_NV21));
    glUniform2f(glGetUniformLocation(shaderProgram, "texSize"), (float)width, (float)height);

    float bitScale = 1.0f;
    if (bitDepth == 10) bitScale = 65535.0f / 1023.0f;
    else if (bitDepth == 12) bitScale = 65535.0f / 4095.0f;

    if (CF_IS_SEMI_PLANAR(format) || CF_IS_PACKED(format)) bitScale = 1.0f;
    glUniform1f(glGetUniformLocation(shaderProgram, "bitScale"), bitScale);
    glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "cs_matrix"), 1, GL_TRUE, bt709);
    draw();
    glFlush();
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

