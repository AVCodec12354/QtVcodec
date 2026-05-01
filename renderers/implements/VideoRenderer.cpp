#include "VideoRenderer.h"

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

inline const char *vertexShader = R"(
    attribute vec4 vertexIn;
    attribute vec2 textureIn;
    varying vec2 textureOut;
    void main(void) {
        gl_Position = vertexIn;
        textureOut = textureIn;
    }
)";

inline const char *fragmentShader = R"(
    varying vec2 textureOut;
    uniform sampler2D tex_y;
    uniform sampler2D tex_u;
    uniform sampler2D tex_v;
    void main(void) {
        float y, u, v;
        y = texture2D(tex_y, textureOut).r * 64.0616;
        u = texture2D(tex_u, textureOut).r * 64.0616 - 0.5;
        v = texture2D(tex_v, textureOut).r * 64.0616 - 0.5;

        y = (y - 0.0627) * 1.1643;
        vec3 rgb;
        rgb.r = y + 1.5748 * v;
        rgb.g = y - 0.1873 * u - 0.4681 * v;
        rgb.b = y + 1.8556 * u;
        gl_FragColor = vec4(rgb, 1.0);
    }
)";

void VideoRenderer::createShader(GLuint &shader, GLuint type, const char* shaderCode) {
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
    initializeOpenGLFunctions();
    createShader(vShader, GL_VERTEX_SHADER, vertexShader);
    createShader(fShader, GL_FRAGMENT_SHADER, fragmentShader);

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

void VideoRenderer::renderFrame(std::shared_ptr <Qv2Buffer> buffer) {
    if (!buffer->graphicBlocks().at(0)->addr(0) || !buffer->graphicBlocks().at(0)->addr(1) || !buffer->graphicBlocks().at(0)->addr(2)) {
        std::cout << "Error: One of the YUV planes is NULL!" << std::endl;
        return;
    }

    if (!buffer || buffer->graphicBlocks().size() == 0) return;
    auto block = buffer->graphicBlocks().at(0);
    int numOfPlane = block->numPlanes();
    int width = block->width();
    int height = block->height();
    int bitDepth = block->bitDepth();

    if (!isInitialized) {
        initOpenGL(numOfPlane);
        updateSurfaceSize(width, height);
    }

    // TODO: CHANGE width, height by PIX_FMT

    // Tránh lỗi sọc hình nếu width không chia hết cho 4
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    const char* names[] = {"tex_y", "tex_u", "tex_v"};
    glUseProgram(shaderProgram);
    for (int i = 0; i < numOfPlane; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, mTextures[i]);

        int pWidth = (i == 0) ? width : width / 2;
        int pHeight = (i == 0) ? height : height / 2;

        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                (bitDepth == 8) ? GL_R8 : GL_R16,
                pWidth,
                pHeight,
                0,
                GL_RED,
                (bitDepth == 8) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT,
                block->addr(i)
        );
        glUniform1i(glGetUniformLocation(shaderProgram, names[i]), i);
    }
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

