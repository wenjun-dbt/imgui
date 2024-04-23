// dear imgui: Platform Backend for wxWidgets
// This needs to be used along with a Renderer (OpenGL for wxWidgets through wxGLCanvas)

// Implemented features:
//  [X] Platform: Partial keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass ImGuiKey values to all key functions e.g. ImGui::IsKeyPressed(ImGuiKey_Space). [Legacy GLUT values will also be supported unless IMGUI_DISABLE_OBSOLETE_KEYIO is set]
// Missing features:
//  [ ] Platform: Missing mouse cursor shape/visibility support.
//  [ ] Platform: Missing gamepad support.

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#define _CRT_SECURE_NO_WARNINGS

#pragma once
#ifndef IMGUI_DISABLE
#include "imgui.h"

#include "wx/window.h"

// Optionally pass a callback to refresh a window on UI interaction (always update 2 frames minimum, even for mouse move!)
IMGUI_IMPL_API bool     ImGui_ImplWX_Init(void (*RefreshCallback)(void));
IMGUI_IMPL_API bool     ImGui_ImplWX_Init();
IMGUI_IMPL_API void     ImGui_ImplWX_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplWX_NewFrame(wxWindow *canvas);

void ImGui_ImplWX_OnKeyDown(wxKeyEvent &event);
void ImGui_ImplWX_OnKeyUp(wxKeyEvent &event);
void ImGui_ImplWX_OnChar(wxKeyEvent &event);
void ImGui_ImplWX_OnMouseMoveEvent(wxMouseEvent &event);
void ImGui_ImplWX_OnMouseWheelEvent(wxMouseEvent &event);
void ImGui_ImplWX_OnMouseLeftEvent(wxMouseEvent &event);
void ImGui_ImplWX_OnMouseRightEvent(wxMouseEvent &event);
void ImGui_ImplWX_OnMouseMiddleEvent(wxMouseEvent &event);

#endif // #ifndef IMGUI_DISABLE
