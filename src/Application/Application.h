#pragma once

#include <SDL3/SDL.h>
#include <cstddef>

#include "Rendering/Shader.h"
#include "Camera/Camera.h"
#include "Scene/Scene.h"

enum class CameraMode {Static, Freecam};

class Application
{
public:
    Application();
    ~Application();

    bool Initialise();
    void Run();
    void Shutdown();

    // Camera Variables
    Camera m_camera;

    CameraMode m_cameraMode = CameraMode::Freecam;

    float m_deltaTime = 0.0f;
    Uint64 m_lastFrameTime = 0;

    Scene m_scene;

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

    void UpdateWindowTitle(std::size_t visibleObjects,std::size_t totalObjects);
};