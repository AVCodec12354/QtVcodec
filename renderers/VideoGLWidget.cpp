#include"VideoGLWidget.h"

VideoGLWidget::~VideoGLWidget() {

}

void VideoGLWidget::setFrameData(const QByteArray &data, int w, int h) {
    mData = data;
    mWidth = w;
    mHeight = h;
    update();
}
void VideoGLWidget::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_TEXTURE_2D);

    const char *vsrc =
            "attribute vec4 vertexIn; \n"
            "attribute vec2 textureIn; \n"
            "varying vec2 textureOut; \n"
            "void main(void) { \n"
            "    gl_Position = vertexIn; \n"
            "    textureOut = textureIn; \n"
            "} \n";

    // FRAGMENT SHADER: Sửa lỗi màu xanh bằng cách nhân hệ số 64.0
    const char *fsrc =
            "varying vec2 textureOut; \n"
            "uniform sampler2D tex_y; \n"
            "uniform sampler2D tex_u; \n"
            "uniform sampler2D tex_v; \n"
            "void main(void) { \n"
            "    float y, u, v; \n"
            "    // Nhân 64.0616 để đưa dải 10-bit về 1.0 (65535/1023) \n"
            "    y = texture2D(tex_y, textureOut).r * 64.0616; \n"
            "    u = texture2D(tex_u, textureOut).r * 64.0616 - 0.5; \n"
            "    v = texture2D(tex_v, textureOut).r * 64.0616 - 0.5; \n"
            "    \n"
            "    // Công thức BT.709 Limited Range chuẩn \n"
            "    y = (y - 0.0627) * 1.1643; \n"
            "    vec3 rgb; \n"
            "    rgb.r = y + 1.5748 * v; \n"
            "    rgb.g = y - 0.1873 * u - 0.4681 * v; \n"
            "    rgb.b = y + 1.8556 * u; \n"
            "    gl_FragColor = vec4(rgb, 1.0); \n"
            "} \n";

    mProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vsrc);
    mProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fsrc);
    mProgram.link();
    glGenTextures(3, mTextures);

    for(int i=0; i<3; i++) {
        glBindTexture(GL_TEXTURE_2D, mTextures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}
void VideoGLWidget::paintGL() {
    if (mData.isEmpty() || mWidth <= 0 || mHeight <=0) return;

    mProgram.bind();
    const unsigned short* pData = (const unsigned short*)mData.data();

    // 4:2:2 10-bit: Y là W*H, U và V là (W/2)*H
    int y_size = mWidth * mHeight;
    int uv_size = (mWidth / 2) * mHeight;

    renderPlane(0, pData, mWidth, mHeight);                        // Y
    renderPlane(1, pData + y_size, mWidth / 2, mHeight);           // U
    renderPlane(2, pData + y_size + uv_size, mWidth / 2, mHeight);  // V

    static const GLfloat vertices[] = { -1,1, 1,1, -1,-1, 1,-1 };
    static const GLfloat texCoords[] = { 0,0, 1,0, 0,1, 1,1 };

    mProgram.setAttributeArray("vertexIn", GL_FLOAT, vertices, 2);
    mProgram.enableAttributeArray("vertexIn");
    mProgram.setAttributeArray("textureIn", GL_FLOAT, texCoords, 2);
    mProgram.enableAttributeArray("textureIn");

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    mProgram.release();
}

void VideoGLWidget::renderPlane(int index, const unsigned short* data, int w, int h){
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, mTextures[index]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, w, h, 0, GL_RED, GL_UNSIGNED_SHORT, data);
    glUniform1i(mProgram.uniformLocation(index == 0 ? "tex_y" : (index == 1 ? "tex_u" : "tex_v")), index);
}