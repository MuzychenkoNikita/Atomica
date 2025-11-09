#include "Interface.hpp"
#include "Core.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <iostream>

void processInput(GLFWwindow* window);

int main() {
    Atomica::InitiateCore();

    Atomica::InitiateInterface();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(Atomica::window)) {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        Atomica::RenderConstructor();
        Atomica::RenderAtom();
        
        Atomica::RenderInterface();
        Atomica::CleanInterface();
        glfwSwapBuffers(Atomica::window);
        glfwPollEvents();
    }

    Atomica::DestroyInterface();

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------

