#pragma once

#include <SDL3/SDL.h>

#include "Rendering/Shader.h"

class Application
{
public:
    Application();
    ~Application();

    bool Initialise();
    void Run();
    void Shutdown();

private:
    void ProcessEvents();
    void Update();
    void Render();

    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;

    GLuint m_triangleVAO = 0;
    GLuint m_triangleVBO = 0;

    Shader m_triangleShader;

    bool m_running = false;

    int m_windowWidth = 1920;
    int m_windowHeight = 1080;
};