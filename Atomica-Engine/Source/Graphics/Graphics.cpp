#include "Graphics.hpp"

namespace Atomica
{
FrameBuffer::FrameBuffer()
    : mFBO(0)
    , mTextureID(0)
    , mSize({1000, 1000})
{
    // Create Frame Buffer
    glGenFramebuffers(1, &mFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    // Create Color Texture
    GenTexture();
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Frame Buffer is NOT complete!" << std::endl;
        }
    // Check FBO for completness
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
FrameBuffer::~FrameBuffer() {
    
}
void FrameBuffer::SetSize(glm::vec2 size) {
    mSize = size;
}
uint32_t FrameBuffer::GetFBO() {
    return mFBO;
}
glm::vec2 FrameBuffer::GetSize() {
    return mSize;
}
uint32_t FrameBuffer::GetTextureID(){
    return mTextureID;
}
void FrameBuffer::GenTexture() {
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mSize.x, mSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureID, 0);
}
void FrameBuffer::BindBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glClear(GL_COLOR_BUFFER_BIT);
}
void FrameBuffer::UnBindBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Shader::Init(GLenum shaderType, const char* shaderCode) {
    mShaderID = glCreateShader(shaderType);
    
    glShaderSource(mShaderID, 1, &shaderCode, NULL);
    glCompileShader(mShaderID);
    
    CheckCompileErrors();
}
unsigned int Shader::GetID() {
    return mShaderID;
}
Shader::~Shader() {
    glDeleteShader(mShaderID);
}
void Shader::CheckCompileErrors() {
    int success;
    char infoLog[1024];
    glGetShaderiv(mShaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(mShaderID, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }
}

void ShaderProgram::Init(std::span<unsigned int> shaderIDs) {
    mProgramID = glCreateProgram();
    for (auto shaderID : shaderIDs) glAttachShader(mProgramID, shaderID);
    glLinkProgram(mProgramID);
    CheckCompileErrors();
}
unsigned int ShaderProgram::GetID() {
    return mProgramID;
}
void ShaderProgram::Use() {
    glUseProgram(mProgramID);
}
ShaderProgram::~ShaderProgram() {
    glDeleteProgram(mProgramID);
}
void ShaderProgram::CheckCompileErrors() {
    int success;
    char infoLog[1024];
    glGetProgramiv(mProgramID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(mProgramID, 1024, NULL, infoLog);
        std::cout << "ERROR::PROGRAM_LINKING_ERROR" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }
}
}
