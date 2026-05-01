#pragma once

#include <string>
#include <Qv2Renderer.h>
#include <Qv2Buffer.h>

#include <QOpenGLFunctions>

class VideoRenderer : public Qv2Renderer, protected QOpenGLFunctions {
public:
    VideoRenderer();
    ~VideoRenderer() override;
    void renderFrame(std::shared_ptr<Qv2Buffer> buffer) override;
private:
    void initOpenGL(int numOfPlane);
    void createShader(GLuint &shader, GLuint type, const char* shaderCode);
    void draw();

    // Full Screen
    const float vertices[8] = {
            -1.0f,  -1.0f,
             1.0f,  -1.0f,
            -1.0f,   1.0f,
             1.0f,   1.0f
    };
    // Correct with vertices
    const float texCoords[8] = {
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f
    };
    bool isInitialized = false;
    GLuint shaderProgram, vShader, fShader;
    GLuint mTextures[MAX_NUM_PLANES] = {0};
};