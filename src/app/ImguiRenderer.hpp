#pragma once

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <vector>
#include "SDL2/SDL.h"

class ImguiRenderer {
public:
    ImguiRenderer(SDL_Window *window, const char *glsl_version);

    void NewFrame();

    virtual void Update();

    static void Shutdown();
private:
    ImGuiIO *io;
};