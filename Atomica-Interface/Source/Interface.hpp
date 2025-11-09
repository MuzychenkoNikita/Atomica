#pragma once

#include "Core.hpp"

#include <GLFW/glfw3.h>

#include <iostream>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

namespace Atomica
{
// Variables
inline ImGuiIO* io = nullptr;
// Windows
inline void AtomWindow() {
    if (ImGui::Begin("Atom Viewport")) {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        const float aspect = Atomica::AtomFrameBuffer->GetSize().x / Atomica::AtomFrameBuffer->GetSize().y;

        float w = avail.x;
        float h = w / aspect;

        if (h > avail.y) {
            h = avail.y;
            w = h * aspect;
        }
        
        // center inside
        ImVec2 cursor = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(cursor.x + (avail.x - w) * 0.5f,
                                                   cursor.y + (avail.y - h) * 0.5f));

        
        ImGui::Image(
            (ImTextureID)(intptr_t)Atomica::AtomFrameBuffer->GetTextureID(),
            ImVec2((float)(int)w, (float)(int)h),
            ImVec2(0.0f, 1.0f),
            ImVec2(1.0f, 0.0f)
        );
    } ImGui::End();
}

inline void ChatWindow() {
    if (ImGui::Begin("Chat")) {
        
    } ImGui::End();
}

inline void ConstructorWindow() {
    if (ImGui::Begin("Constructor")) {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        const float aspect = Atomica::ConstructorFrameBuffer->GetSize().x / Atomica::ConstructorFrameBuffer->GetSize().y;

        float w = avail.x;
        float h = w / aspect;

        if (h > avail.y) {
            h = avail.y;
            w = h * aspect;
        }
        
        // center inside
        ImVec2 cursor = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(cursor.x + (avail.x - w) * 0.5f,
                                                   cursor.y + (avail.y - h) * 0.5f));
        
        ImGui::Image(
            (ImTextureID)(intptr_t)Atomica::ConstructorFrameBuffer->GetTextureID(),
            ImVec2((float)(int)w, (float)(int)h),
            ImVec2(0.0f, 1.0f),
            ImVec2(1.0f, 0.0f)
        );
    } ImGui::End();
}

// Functions
inline void InitiateInterface() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO(); (void)io;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    #ifdef __EMSCRIPTEN__
        ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
    #endif
        ImGui_ImplOpenGL3_Init("#version 330");
}
inline void RenderInterface() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();
    
    AtomWindow();
    ChatWindow();
    ConstructorWindow();
    
    ImGui::ShowDemoWindow();
    ImGui::Render();
    
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}



inline void CleanInterface() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

inline void DestroyInterface() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
}
