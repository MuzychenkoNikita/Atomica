#pragma once

#include "Graphics.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Atomica 
{
// Functions
void InitiateCore();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);



void RenderConstructor();
void RenderAtom();

void InitConstructorShader();
void InitAtomShader();

// Variables
extern GLFWwindow* window;
extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;

extern Atomica::FrameBuffer* ConstructorFrameBuffer;
extern Atomica::FrameBuffer* AtomFrameBuffer;

extern Atomica::Shader* AtomFrag;
extern Atomica::Shader* AtomVert;
extern Atomica::ShaderProgram* AtomShader;

extern Atomica::Shader* ConstructorFrag;
extern Atomica::Shader* ConstructorVert;
extern Atomica::ShaderProgram* ConstructorShader;

extern GLuint quadVAO;
extern GLuint quadVBO;

extern int numProtons;
extern int numNeutrons;
extern int eCount[7];
}
