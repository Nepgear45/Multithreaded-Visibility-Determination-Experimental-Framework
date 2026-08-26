#pragma once

#include <SDL3/SDL.h>

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

private:
    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;

    bool m_running = false;

    int m_windowWidth = 1920;
    int m_windowHeight = 1080;
};