#include "Core.hpp"

#include <iostream>

namespace Atomica 
{
// Functions
void InitiateCore() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Atomica", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // My FrameBuffers
    ConstructorFrameBuffer = new Atomica::FrameBuffer();
    AtomFrameBuffer = new Atomica::FrameBuffer();
    
    InitConstructorShader();
    InitAtomShader();
    
    // fullscreen quad (triangle strip)
    float quadVertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void RenderConstructor() {
    ConstructorFrameBuffer->BindBuffer();
    glClearColor(0.2, 0.2, 1., 1.);
    glClear(GL_COLOR_BUFFER_BIT);
    ConstructorFrameBuffer->UnBindBuffer();
}

void RenderAtom() {
    AtomFrameBuffer->BindBuffer();
    glClearColor(0, 0.2, 0, 1.);
    glClear(GL_COLOR_BUFFER_BIT);

    unsigned int shaderID = AtomShader->GetID();
    glUseProgram(shaderID); // <<< MUST come before glUniform calls

    glUniform1f(glGetUniformLocation(shaderID, "iTime"), glfwGetTime());
    glUniform2f(glGetUniformLocation(shaderID, "iResolution"), 1000, 1000);
    glUniform1i(glGetUniformLocation(shaderID, "numProtons"), 42);
    glUniform1i(glGetUniformLocation(shaderID, "numNeutrons"), 42);

    int electrons[7] = {2, 3, 5, 0, 0, 0, 0};
    glUniform1iv(glGetUniformLocation(shaderID, "eCount"), 7, electrons);

    glUniform1f(glGetUniformLocation(shaderID, "baseRadius"), 0.2f);
    glUniform1f(glGetUniformLocation(shaderID, "orbitGap"), 0.2f);
    glUniform1f(glGetUniformLocation(shaderID, "electronRadius"), 0.04f);
    glUniform1f(glGetUniformLocation(shaderID, "ringTube"), 0.005f);
    glUniform1f(glGetUniformLocation(shaderID, "particleRadius"), 0.08f);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    glUseProgram(0);
    AtomFrameBuffer->UnBindBuffer();
}

// Variables
GLFWwindow* window = nullptr;
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

Atomica::FrameBuffer* ConstructorFrameBuffer = nullptr;
Atomica::FrameBuffer* AtomFrameBuffer = nullptr;

GLuint quadVAO = 0;
GLuint quadVBO = 0;
}
