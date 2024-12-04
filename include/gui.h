#define GUI_H_

#include "./imgui/imgui.h"
#include "./imgui/imgui_impl_glfw.h"
#include "./imgui/imgui_impl_opengl3.h"

bool UIContext_init();
void UIContext_pre_render();
void UIContext_post_render();
void UIContext_end();
