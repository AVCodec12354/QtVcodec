#include "VideoRenderer.h"

#include <Program.h>
#include <Qv2Constants.h>

#define MAX_LOG_LENGTH 512

VideoRenderer::VideoRenderer() {
    std::cout << __FUNCTION__  << std::endl;
    mIsInitialized = false;
}

VideoRenderer::~VideoRenderer() {
    std::cout << __FUNCTION__  << std::endl;
    if (QOpenGLContext::currentContext() && mIsInitialized) {
        mWidget->makeCurrent();
        glDeleteTextures(MAX_NUM_PLANES, mTextures);
        glDeleteProgram(mShaderProgram);
        mWidget->doneCurrent();
    }
    mIsInitialized = false;
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
    createShader(mVertexShader, GL_VERTEX_SHADER, vertexShader);
    std::string fragmentShader;
    generateFragmentShader(fragmentShader, numOfPlane);
    std::cout << "Shader code:\n" << fragmentShader << std::endl;
    createShader(mFragmentShader, GL_FRAGMENT_SHADER, fragmentShader.c_str());

    mShaderProgram = glCreateProgram();
    glAttachShader(mShaderProgram, mVertexShader);
    glAttachShader(mShaderProgram, mFragmentShader);
    glLinkProgram(mShaderProgram);

    int success;
    char infoLog[MAX_LOG_LENGTH];
    glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(mShaderProgram, MAX_LOG_LENGTH, nullptr, infoLog);
        std::cout << "Program linking error: " << infoLog << std::endl;
    }
    glDeleteShader(mVertexShader);
    glDeleteShader(mFragmentShader);

    glGenTextures(numOfPlane, mTextures);
    for(int i = 0; i < numOfPlane; i++) {
        glBindTexture(GL_TEXTURE_2D, mTextures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    mIsInitialized = true;
}

void VideoRenderer::renderFrame(std::shared_ptr<Qv2Buffer> buffer) {
    if (!buffer || buffer->graphicBlocks().empty()) return;
    auto block = buffer->graphicBlocks()[0];
    int format    = block->format();
    int width     = block->width();
    int height    = block->height();
    int bitDepth  = block->bitDepth();
    int numPlanes = block->numPlanes();

    if (mIsInitialized &&
        (mCurrentNumPlanes != numPlanes ||
        mCurrentWidth != width ||
        mCurrentHeight != height)
    ) {
        mWidget->makeCurrent();
        glDeleteTextures(mCurrentNumPlanes, mTextures);
        glDeleteProgram(mShaderProgram);
        mIsInitialized = false;
    }

    if (!mIsInitialized) {
        initOpenGL(numPlanes);
        QMetaObject::invokeMethod(mWidget, [this, width, height]() {
            this->updateSurfaceSize(width, height);
        }, Qt::QueuedConnection);

        mCurrentNumPlanes = numPlanes;
        mCurrentWidth = width;
        mCurrentHeight = height;
    }
    glViewport(0, 0, mWidget->width(), mWidget->height());

    glUseProgram(mShaderProgram);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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
        } else if (CF_IS_SEMI_PLANAR(format) && i == 1) {
            // Semi-planar (NV12, P010...)
            internalFormat = (bitDepth <= 8) ? GL_RG8 : GL_RG16;
            glFormat = GL_RG;
        } else {
            // Planar
            internalFormat = (bitDepth <= 8) ? GL_R8 : GL_R16;
            glFormat = GL_RED;
        }

        int bytesPerComponent = (bitDepth <= 8) ? 1 : 2;
        int componentsPerPixel = (glFormat == GL_RG) ? 2 : (glFormat == GL_RGBA ? 4 : 1);
        int bpp = bytesPerComponent * componentsPerPixel;
        if (block->stride(i) < pW * bpp) {
            std::cerr << "Warning: Stride is not a multiple of pixel size!" << std::endl;
            continue;
        }
        glPixelStorei(GL_UNPACK_ROW_LENGTH, block->stride(i) / bpp);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, pW, pH, 0, glFormat, type, block->addr(i));
        std::string samplerName = "tex" + std::to_string(i);
        glUniform1i(glGetUniformLocation(mShaderProgram, samplerName.c_str()), i);
    }
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glUniform1i(glGetUniformLocation(mShaderProgram, "isLimitedRange"), (block->getColorRange() == QV2_CR_LIMITED));
    glUniform1i(glGetUniformLocation(mShaderProgram, "isPacked"), CF_IS_PACKED(format));
    glUniform1i(glGetUniformLocation(mShaderProgram, "isSemiPlanar"), CF_IS_SEMI_PLANAR(format));
    glUniform1i(glGetUniformLocation(mShaderProgram, "swapUV"), (format == QV2_CF_NV21));
    glUniform2f(glGetUniformLocation(mShaderProgram, "texSize"), (float)width, (float)height);

    float bitScale = 1.0f;
    if (bitDepth == 10) bitScale = 65535.0f / 1023.0f;
    else if (bitDepth == 12) bitScale = 65535.0f / 4095.0f;
    if (CF_IS_SEMI_PLANAR(format) || CF_IS_PACKED(format)) bitScale = 1.0f;
    glUniform1f(glGetUniformLocation(mShaderProgram, "bitScale"), bitScale);

    GLint matrixLoc = glGetUniformLocation(mShaderProgram, "cs_matrix");
    if (block->getColorMatrix() == QV2_CM_BT2020NC ||
        block->getColorMatrix() == QV2_CM_BT2020C
    ) {
        std::cout << "Using BT2020" << std::endl;
        glUniformMatrix3fv(matrixLoc, 1, GL_TRUE, bt2020);
    } else {
        std::cout << "Using BT709 (Default)" << std::endl;
        glUniformMatrix3fv(matrixLoc, 1, GL_TRUE, bt709);
    }

    draw();
    glFlush();
    glUseProgram(0);
    for (int i = 0; i < numPlanes; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void VideoRenderer::draw() {
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLint vertexInLoc = glGetAttribLocation(mShaderProgram, "vertexIn");
    glEnableVertexAttribArray(vertexInLoc);
    glVertexAttribPointer(vertexInLoc, 2, GL_FLOAT, GL_FALSE, 0, vertices);

    GLint textureInLoc = glGetAttribLocation(mShaderProgram, "textureIn");
    glEnableVertexAttribArray(textureInLoc);
    glVertexAttribPointer(textureInLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(vertexInLoc);
    glDisableVertexAttribArray(textureInLoc);
}

