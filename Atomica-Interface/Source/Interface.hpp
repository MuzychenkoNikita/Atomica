#pragma once

#include "Core.hpp"
#include "Judge.hpp"
#include "Gemini.hpp"

#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstring>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

namespace Atomica
{
// Variables
inline ImGuiIO* io = nullptr;
// Windows
inline float ImLengthSqr(const ImVec2& a, const ImVec2& b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy;
}
inline float ImDist(const ImVec2& a, const ImVec2& b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

inline void AtomWindow() {
    static bool showDescription = true; // toggle state

    if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent, false)) { // false = no repeat
        showDescription = !showDescription;
    }

    if (ImGui::Begin("Atom Viewport")) {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        const float aspect = Atomica::AtomFrameBuffer->GetSize().x / Atomica::AtomFrameBuffer->GetSize().y;

        float w = avail.x;
        float h = w / aspect;
        if (h > avail.y) { h = avail.y; w = h * aspect; }

        ImVec2 cursor = ImGui::GetCursorPos();
        ImVec2 imagePos(cursor.x + (avail.x - w) * 0.5f,
                        cursor.y + (avail.y - h) * 0.5f);

        ImGui::SetCursorPos(imagePos);
        ImGui::Image(
            (ImTextureID)(intptr_t)Atomica::AtomFrameBuffer->GetTextureID(),
            ImVec2(w, h),
            ImVec2(0.0f, 1.0f),
            ImVec2(1.0f, 0.0f)
        );

        if (showDescription) {
            std::string info = Atomica::describeAtom(Atomica::numProtons, Atomica::numNeutrons, Atomica::eCount);


            ImVec2 maxTextSize = ImVec2(0, 0);
            const float lineHeight = ImGui::GetTextLineHeightWithSpacing();
            size_t start = 0, end;
            while ((end = info.find('\n', start)) != std::string::npos) {
                std::string line = info.substr(start, end - start);
                ImVec2 lineSize = ImGui::CalcTextSize(line.c_str());
                maxTextSize.x = std::max(maxTextSize.x, lineSize.x);
                maxTextSize.y += lineHeight;
                start = end + 1;
            }

            ImVec2 windowPos = ImGui::GetWindowPos();
            const float padding = 10.0f;
            ImVec2 textStart = ImVec2(
                windowPos.x + imagePos.x + padding,
                windowPos.y + imagePos.y + h - maxTextSize.y - padding
            );

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImU32 textColor = IM_COL32(255, 255, 255, 230);
            ImU32 bgColor   = IM_COL32(0, 0, 0, 120);

            drawList->AddRectFilled(
                ImVec2(textStart.x - 5, textStart.y - 5),
                ImVec2(textStart.x + maxTextSize.x + 10, textStart.y + maxTextSize.y + 10),
                bgColor, 8.0f
            );

            start = 0;
            float lineY = textStart.y;
            while ((end = info.find('\n', start)) != std::string::npos) {
                std::string line = info.substr(start, end - start);
                drawList->AddText(ImVec2(textStart.x, lineY), textColor, line.c_str());
                lineY += lineHeight;
                start = end + 1;
            }
        }
    }
    ImGui::End();
}

inline void ChatWindow() {
    static std::vector<std::pair<std::string, bool>> chatLog;
    static std::mutex chatMutex;
    static char inputBuf[512] = "";
    static bool isLoading = false;
    static int eCount[7] = {0};
    static size_t lastMessageCount = 0;
    static bool scrollToBottom = false;

    ImGui::SetNextWindowSize(ImVec2(520, 460), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Ask Gemini", nullptr, ImGuiWindowFlags_NoCollapse)) {

        ImVec2 winSize = ImGui::GetContentRegionAvail();

        ImGui::BeginChild("ChatScroll", ImVec2(0, winSize.y - 60), true,
                          ImGuiWindowFlags_AlwaysVerticalScrollbar);
        {
            std::lock_guard<std::mutex> lock(chatMutex);
            float maxWidth = ImGui::GetContentRegionAvail().x * 0.75f;

            for (const auto& [msg, isUser] : chatLog) {
                ImGui::PushID(&msg);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);

                ImVec4 bubbleColor = isUser
                    ? ImVec4(0.2f, 0.4f, 0.9f, 0.6f)
                    : ImVec4(0.25f, 0.6f, 0.3f, 0.6f);

                ImVec2 textSize = ImGui::CalcTextSize(msg.c_str(), nullptr, false, maxWidth);
                ImVec2 bubbleSize(textSize.x + 20, textSize.y + 16);

                float cursorX = isUser
                    ? ImGui::GetWindowContentRegionMax().x - bubbleSize.x - 20
                    : 10.0f;
                ImGui::SetCursorPosX(cursorX);

                ImGui::PushStyleColor(ImGuiCol_ChildBg, bubbleColor);
                ImGui::BeginChild(ImGui::GetID(msg.c_str()), bubbleSize, true,
                                  ImGuiWindowFlags_AlwaysUseWindowPadding);
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + maxWidth);
                ImGui::TextWrapped("%s", msg.c_str());
                ImGui::PopTextWrapPos();

                if (ImGui::BeginPopupContextWindow()) {
                    if (ImGui::MenuItem("Copy message")) {
                        ImGui::SetClipboardText(msg.c_str());
                    }
                    ImGui::EndPopup();
                }

                ImGui::EndChild();
                ImGui::PopStyleColor();
                ImGui::PopStyleVar(2);
                ImGui::PopID();

                ImGui::Dummy(ImVec2(0, 6));
            }

            if (isLoading) {
                ImGui::SetCursorPosX(10);
                ImGui::TextDisabled("Gemini is thinking...");
            }

            if (scrollToBottom) {
                ImGui::SetScrollHereY(1.0f);
                scrollToBottom = false;
            }
        }
        ImGui::EndChild();

        ImGui::Separator();
        ImGui::PushItemWidth(-60);
        bool send = ImGui::InputText("##chatinput", inputBuf, IM_ARRAYSIZE(inputBuf),
                                     ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        send |= ImGui::Button("Send");

        if (send && inputBuf[0] != '\0' && !isLoading) {
            std::string question = inputBuf;
            {
                std::lock_guard<std::mutex> lock(chatMutex);
                chatLog.emplace_back(question, true);
            }
            inputBuf[0] = '\0';
            isLoading = true;
            scrollToBottom = true;

            auto chatLogPtr = &chatLog;
            auto chatMutexPtr = &chatMutex;
            std::string apiKeyCopy = Atomica::apiKey;
            int numProtonsCopy = Atomica::numProtons;
            int numNeutronsCopy = Atomica::numNeutrons;
            int eCountCopy[7];
            memcpy(eCountCopy, eCount, sizeof(eCount));

            std::thread([question, chatLogPtr, chatMutexPtr, apiKeyCopy,
                         numProtonsCopy, numNeutronsCopy, eCountCopy]() mutable {
                std::string atomInfo = describeAtom(numProtonsCopy, numNeutronsCopy, eCountCopy);
                std::string answer = askGemini(apiKeyCopy, atomInfo, question);

                {
                    std::lock_guard<std::mutex> lock(*chatMutexPtr);
                    chatLogPtr->emplace_back(answer, false);
                }
                isLoading = false;
                scrollToBottom = true;
            }).detach();
        }

        if (chatLog.size() != lastMessageCount) {
            scrollToBottom = true;
            lastMessageCount = chatLog.size();
        }
    }
    ImGui::End();
}


inline void ConstructorWindow() {
    if (ImGui::Begin("Constructor", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
    {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 center = ImVec2(winPos.x + avail.x * 0.5f, winPos.y + avail.y * 0.55f);
        ImDrawList* draw = ImGui::GetWindowDrawList();

        float baseScale = std::min(avail.x, avail.y) / 600.0f;
        int maxElectronsPerShell[7] = { 2, 8, 18, 32, 50, 72, 98 };

        ImVec2 clearBtnPos(winPos.x + avail.x - 65 * baseScale, winPos.y + 20 * baseScale);
        ImGui::SetCursorScreenPos(clearBtnPos);
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(150, 30, 30, 200));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(200, 50, 50, 220));
        if (ImGui::Button("Clear", ImVec2(70 * baseScale, 25 * baseScale))) {
            numProtons = 1;
            numNeutrons = 0;
            for (int i = 0; i < 7; ++i) eCount[i] = 0;
            eCount[0] = 1;
        }
        ImGui::PopStyleColor(2);

        for (int i = 0; i < 7; ++i) {
            float radius = baseScale * (70.0f + 35.0f * i);
            draw->AddCircle(center, radius, IM_COL32(255, 255, 255, 80), 128, 1.2f);
        }

        draw->AddCircleFilled(center, 50.0f * baseScale, IM_COL32(80, 80, 80, 255));

        static std::vector<ImVec2> protonPositions;
        static std::vector<ImVec2> neutronPositions;

        if (protonPositions.size() != numProtons)
            protonPositions.resize(numProtons);
        if (neutronPositions.size() != numNeutrons)
            neutronPositions.resize(numNeutrons);

        for (int i = 0; i < numProtons; i++) {
            float ang = (2 * M_PI * i) / std::max(numProtons, 1);
            ImVec2 pos(center.x + cos(ang) * 20 * baseScale, center.y + sin(ang) * 20 * baseScale);
            protonPositions[i] = pos;
            draw->AddCircleFilled(pos, 6.0f * baseScale, IM_COL32(255, 60, 60, 255));
            ImVec2 ts = ImGui::CalcTextSize("+");
            draw->AddText(ImVec2(pos.x - ts.x * 0.5f, pos.y - ts.y * 0.5f),
                          IM_COL32(255, 255, 255, 255), "+");
        }

        for (int i = 0; i < numNeutrons; i++) {
            float ang = (2 * M_PI * i) / std::max(numNeutrons, 1);
            ImVec2 pos(center.x + cos(ang + 0.3f) * 30 * baseScale, center.y + sin(ang + 0.3f) * 30 * baseScale);
            neutronPositions[i] = pos;
            draw->AddCircleFilled(pos, 6.0f * baseScale, IM_COL32(255, 255, 255, 255));
            ImVec2 ts = ImGui::CalcTextSize("n");
            draw->AddText(ImVec2(pos.x - ts.x * 0.5f, pos.y - ts.y * 0.5f),
                          IM_COL32(50, 50, 50, 255), "n");
        }

        for (int r = 0; r < 7; ++r) {
            int count = eCount[r];
            float radius = baseScale * (70.0f + 35.0f * r);
            for (int e = 0; e < count; ++e) {
                float ang = (2 * M_PI * e) / std::max(count, 1);
                ImVec2 pos(center.x + cos(ang) * radius, center.y + sin(ang) * radius);
                draw->AddCircleFilled(pos, 5.0f * baseScale, IM_COL32(100, 180, 255, 255));
                ImVec2 ts = ImGui::CalcTextSize("-");
                draw->AddText(ImVec2(pos.x - ts.x * 0.5f, pos.y - ts.y * 0.5f),
                              IM_COL32(255, 255, 255, 255), "-");
            }
        }

        float panelWidth = 160 * baseScale;
        float panelHeight = 60 * baseScale;
        ImVec2 panelPos(winPos.x + avail.x - panelWidth - 20 * baseScale,
                        winPos.y + avail.y - panelHeight - 20 * baseScale);

        draw->AddRectFilled(panelPos,
                            ImVec2(panelPos.x + panelWidth, panelPos.y + panelHeight),
                            IM_COL32(25, 25, 25, 230), 10.0f);

        ImVec2 electronPos(panelPos.x + 30 * baseScale, panelPos.y + panelHeight * 0.5f);
        ImVec2 protonPos  (panelPos.x + 80 * baseScale, panelPos.y + panelHeight * 0.5f);
        ImVec2 neutronPos (panelPos.x + 130 * baseScale, panelPos.y + panelHeight * 0.5f);

        auto CenterText = [&](ImVec2 center, const char* text, ImU32 col) {
            ImVec2 ts = ImGui::CalcTextSize(text);
            draw->AddText(ImVec2(center.x - ts.x * 0.5f, center.y - ts.y * 0.5f), col, text);
        };

        draw->AddCircleFilled(electronPos, 11 * baseScale, IM_COL32(100, 180, 255, 255));
        draw->AddCircleFilled(protonPos,   11 * baseScale, IM_COL32(255, 80, 80, 255));
        draw->AddCircleFilled(neutronPos,  11 * baseScale, IM_COL32(255, 255, 255, 255));

        CenterText(electronPos, "-", IM_COL32(255, 255, 255, 255));
        CenterText(protonPos,   "+", IM_COL32(255, 255, 255, 255));
        CenterText(neutronPos,  "n", IM_COL32(50, 50, 50, 255));

        static int draggingType = -1;
        static int sourceRing = -1;
        static int draggingIndex = -1;
        ImVec2 m = ImGui::GetMousePos();

        auto dist2 = [](ImVec2 a, ImVec2 b) {
            float dx = a.x - b.x, dy = a.y - b.y;
            return dx * dx + dy * dy;
        };

        if (ImGui::IsMouseClicked(0)) {
            if (dist2(m, electronPos) < (11 * baseScale) * (11 * baseScale)) draggingType = 0;
            else if (dist2(m, protonPos) < (11 * baseScale) * (11 * baseScale)) draggingType = 1;
            else if (dist2(m, neutronPos) < (11 * baseScale) * (11 * baseScale)) draggingType = 2;
            else {
                for (int r = 0; r < 7; ++r) {
                    int count = eCount[r];
                    float radius = baseScale * (70.0f + 35.0f * r);
                    for (int e = 0; e < count; ++e) {
                        float ang = (2 * M_PI * e) / std::max(count, 1);
                        ImVec2 pos(center.x + cos(ang) * radius, center.y + sin(ang) * radius);
                        if (dist2(m, pos) < (10 * baseScale) * (10 * baseScale)) {
                            draggingType = 0;
                            sourceRing = r;
                            eCount[r] = std::max(0, eCount[r] - 1);
                            goto drag_found;
                        }
                    }
                }

                for (int i = 0; i < (int)protonPositions.size(); ++i) {
                    if (dist2(m, protonPositions[i]) < (10 * baseScale) * (10 * baseScale)) {
                        draggingType = 1;
                        draggingIndex = i;
                        protonPositions.erase(protonPositions.begin() + i);
                        numProtons = std::max(0, numProtons - 1);
                        goto drag_found;
                    }
                }

                for (int i = 0; i < (int)neutronPositions.size(); ++i) {
                    if (dist2(m, neutronPositions[i]) < (10 * baseScale) * (10 * baseScale)) {
                        draggingType = 2;
                        draggingIndex = i;
                        neutronPositions.erase(neutronPositions.begin() + i);
                        numNeutrons = std::max(0, numNeutrons - 1);
                        goto drag_found;
                    }
                }
            }
        }
    drag_found:;

        if (ImGui::IsMouseDown(0) && draggingType >= 0) {
            ImU32 color =
                draggingType == 0 ? IM_COL32(100,180,255,180) :
                draggingType == 1 ? IM_COL32(255,80,80,180) :
                                    IM_COL32(255,255,255,180);
            draw->AddCircleFilled(m, 16 * baseScale, color);

            const char* label = (draggingType == 0 ? "-" :
                                (draggingType == 1 ? "+" : "n"));
            ImVec2 ts = ImGui::CalcTextSize(label);
            draw->AddText(ImVec2(m.x - ts.x * 0.5f, m.y - ts.y * 0.5f),
                          IM_COL32(255, 255, 255, 255), label);
        }

        if (ImGui::IsMouseReleased(0) && draggingType >= 0) {
            float dx = m.x - center.x, dy = m.y - center.y;
            float dist = sqrtf(dx * dx + dy * dy);
            bool placed = false;

            if (draggingType == 0) {
                for (int i = 0; i < 7; ++i) {
                    float ringR = baseScale * (70.0f + 35.0f * i);
                    if (fabs(dist - ringR) < 20.0f * baseScale) {
                        if (eCount[i] < maxElectronsPerShell[i]) {
                            eCount[i]++;
                            placed = true;
                        } else {
                            ImGui::OpenPopup("Shell Full");
                        }
                        break;
                    }
                }
            } else if (dist < 60.0f * baseScale) {
                if (draggingType == 1) numProtons++;
                else if (draggingType == 2) numNeutrons++;
                placed = true;
            }

            draggingType = -1;
            sourceRing = -1;
            draggingIndex = -1;
        }

        if (ImGui::BeginPopup("Shell Full")) {
            ImGui::Text("This energy level is already full!");
            ImGui::EndPopup();
        }

        ImGui::End();
    }
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
    
    //ImGui::ShowDemoWindow();
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
