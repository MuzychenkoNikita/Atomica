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
    curl_global_init(CURL_GLOBAL_DEFAULT);
    Atomica::InitiateCore();

    Atomica::InitiateInterface();

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
    curl_global_cleanup();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

