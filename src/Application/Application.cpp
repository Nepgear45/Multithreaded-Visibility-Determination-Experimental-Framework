#include "Application.h"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include "System/CPUInfo.h"
#include "Visibility/Frustum.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <string>

Application::Application()
{

}

Application::~Application()
{
    Shutdown();
}

bool Application::Initialise()
{
    // Initialise SDL
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL initialisation failed: " << SDL_GetError() << '\n';

        return false;
    }

    // Configure OpenGL
    SDL_GL_SetAttribute
    (
        SDL_GL_CONTEXT_MAJOR_VERSION,
        4
    );


    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_MINOR_VERSION,
        6
    );

    SDL_GL_SetAttribute
    (
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE
    );

    SDL_GL_SetAttribute
    (
        SDL_GL_DOUBLEBUFFER,
        1
    );

    // Create Window
    m_window = SDL_CreateWindow
    (
        "Multithreaded Visibility Framework",
        m_windowWidth,
        m_windowHeight,
        SDL_WINDOW_OPENGL
    );

    if (m_window == nullptr)
    {
        std::cerr << "Window creation failed: " << SDL_GetError() << '\n';

        SDL_Quit();
        return false;
    }

    // Create OpenGL Context
    m_glContext = SDL_GL_CreateContext(m_window);

    if (m_glContext == nullptr)
    {
        std::cerr << "OpenGL context creation failed: " << SDL_GetError() << '\n';

        SDL_DestroyWindow(m_window);
        m_window = nullptr;

        SDL_Quit();
        return false;
    }

    // Create the Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForOpenGL(m_window, m_glContext);
    ImGui_ImplOpenGL3_Init("#version 460");

    // Load OpenGL Functions using GLAD
    int version = gladLoadGL
    (
        SDL_GL_GetProcAddress
    );

    if (version == 0)
    {
        std::cerr << "Failed to initialise GLAD." << '\n';

        SDL_GL_DestroyContext(m_glContext);
        m_glContext = nullptr;

        SDL_DestroyWindow(m_window);
        m_window = nullptr;

        SDL_Quit();
        return false;
    }

    // Display CPU Information
    CPUInfo::Print();

    // Display OpenGL Information
    std::cout << "\n--- OpenGL Information ---\n";
    std::cout << "GLAD loaded OpenGL " << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << '\n';
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << '\n';
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << '\n';
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << '\n';
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';
    std::cout << "OpenGL " << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << " loaded successfully." << '\n';

    // Enable Depth test
    glEnable(GL_DEPTH_TEST);

    // Configure Viewport
    glViewport
    (
        0,
        0,
        m_windowWidth,
        m_windowHeight
    );

    // Load Cube Shader (keeping old names becuase they will be replaced again anyway)
    if (!m_triangleShader.LoadFromFiles("shaders/triangle.vert", "shaders/triangle.frag"))
    {
        std::cerr << "Failed to load triangle shader." << '\n';
        return false;
    }

    // Cube Geometry
    const float vertices[] =
    {
        // Back face
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,

         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        // Front face
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        // Left face
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

        // Right face
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,

         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

         // Bottom face
         -0.5f, -0.5f, -0.5f,
          0.5f, -0.5f, -0.5f,
          0.5f, -0.5f,  0.5f,

          0.5f, -0.5f,  0.5f,
         -0.5f, -0.5f,  0.5f,
         -0.5f, -0.5f, -0.5f,

         // Top face
         -0.5f,  0.5f, -0.5f,
          0.5f,  0.5f, -0.5f,
          0.5f,  0.5f,  0.5f,

          0.5f,  0.5f,  0.5f,
         -0.5f,  0.5f,  0.5f,
         -0.5f,  0.5f, -0.5f
    };

    // Create VAO / VBO
    glGenVertexArrays
    (
        1,
        &m_triangleVAO
    );

    glGenBuffers
    (
        1,
        &m_triangleVBO
    );


    glBindVertexArray
    (
        m_triangleVAO
    );

    glBindBuffer
    (
        GL_ARRAY_BUFFER,
        m_triangleVBO
    );

    glBufferData
    (
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );

    // Vertex Position Attribute
    glVertexAttribPointer
    (
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        nullptr
    );

    glEnableVertexAttribArray(0);

    // Unbind
    glBindBuffer
    (
        GL_ARRAY_BUFFER,
        0
    );

    glBindVertexArray(0);

    // Scene generation
    m_scene.GenerateGrid
    (
        12,
        12,
        12,
        5.0f
    );

    // Enable VSync
    if (!SDL_GL_SetSwapInterval(1))
    {
        std::cerr << "Warning: VSync could not be enabled: " << SDL_GetError() << '\n';
    }

    // Disable mouse hitting the edge of the window
    SDL_SetWindowRelativeMouseMode(m_window, true);

    // Get the maximum number of concurrent threads reported by the CPU
    m_maxThreadCount = static_cast<std::size_t>(std::thread::hardware_concurrency());

    // Temp
    std::cout << "Maximum Hardware Threads: " << m_maxThreadCount << '\n';

    // hardware_concurrency can return 0 if the value cannot be determined
    if (m_maxThreadCount == 0) m_maxThreadCount = 2;

    // Application State
    m_running = true;
    std::cout << '\n' << "Application initialised successfully." << '\n';

    return true;
}

void Application::Run()
{
    m_lastFrameTime = SDL_GetPerformanceCounter();

    while (m_running)
    {
        const Uint64 currentFrameTime = SDL_GetPerformanceCounter();

        m_deltaTime = static_cast<float>(currentFrameTime - m_lastFrameTime) / static_cast<float>( SDL_GetPerformanceFrequency());
        m_lastFrameTime = currentFrameTime;

        ProcessEvents();
        Update();
        Render();
    }
}

void Application::ProcessEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        // Allow Dear ImGui to process SDL input events
        ImGui_ImplSDL3_ProcessEvent(&event);

        if (event.type == SDL_EVENT_QUIT)
        {
            m_running = false;
        }

        if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            if (m_cameraMode == CameraMode::Freecam)
            {
                const float xOffset = event.motion.xrel;
                const float yOffset = -event.motion.yrel;

                m_camera.ProcessMouseMovement(xOffset, yOffset);
            }
        }

        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            // F1 - Toggle freecam
            if (event.key.scancode == SDL_SCANCODE_F1)
            {
                if (m_cameraMode == CameraMode::Freecam)
                {
                    m_cameraMode = CameraMode::Static;
                    SDL_SetWindowRelativeMouseMode(m_window, false);
                    std::cout << "Camera Mode: Static\n";
                }
                else
                {
                    m_cameraMode = CameraMode::Freecam;
                    SDL_SetWindowRelativeMouseMode(m_window, true);
                    std::cout << "Camera Mode: Freecam\n";
                }
            }

            // F2 - Cycle through culling configurations
            if (event.key.key == SDLK_F2 && !event.key.repeat) CycleCullingMode();
        }
    }
}

void Application::Update()
{
    if (m_cameraMode != CameraMode::Freecam) return;

    const bool* keyboardState = SDL_GetKeyboardState(nullptr);

    if (keyboardState[SDL_SCANCODE_W]) m_camera.ProcessKeyboard(CameraMovement::Forward, m_deltaTime);
    if (keyboardState[SDL_SCANCODE_S]) m_camera.ProcessKeyboard(CameraMovement::Backward, m_deltaTime);
    if (keyboardState[SDL_SCANCODE_A]) m_camera.ProcessKeyboard(CameraMovement::Left, m_deltaTime);
    if (keyboardState[SDL_SCANCODE_D]) m_camera.ProcessKeyboard(CameraMovement::Right, m_deltaTime);
    if (keyboardState[SDL_SCANCODE_SPACE]) m_camera.ProcessKeyboard(CameraMovement::Up, m_deltaTime);
    if (keyboardState[SDL_SCANCODE_LCTRL]) m_camera.ProcessKeyboard(CameraMovement::Down, m_deltaTime);

    /*
        W       forward
        S       backward
        A       left
        D       right
        Space   up
        Ctrl    down
    */
}

void Application::Render()
{
    // Start measuring total frame time
    const auto frameStartTime = std::chrono::high_resolution_clock::now();

    // Get Object Count
    const std::size_t totalObjects = m_scene.GetObjects().size();

    // Clear Buffers
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate camera matrices
    const glm::mat4 view = m_camera.GetViewMatrix();
    const glm::mat4 projection = glm::perspective
    (
        glm::radians(60.0f),
        static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight),
        0.1f,
        1000.0f
    );

    // Build the view-projection matrix
    const glm::mat4 viewProjection = projection * view;

    // Extract the camera frustum ONCE per frame
    const Frustum frustum = Frustum::FromViewProjection(viewProjection);

    // --------------------------------------------- Culling starts ---------------------------------------------

    // Start measuring visibility determination time
    const auto cullingStartTime = std::chrono::high_resolution_clock::now();

    // Run the currently selected visibility determination method
    if (m_cullingMode == CullingMode::SingleThreaded)
    {
        m_singleThreadedCuller.Cull
        (
            m_scene.GetObjects(),
            frustum,
            m_visibleObjects
        );
    }
    else
    {
        m_multithreadedCuller.Cull
        (
            m_scene.GetObjects(),
            frustum,
            m_visibleObjects,
            m_threadCount
        );
    }

    // Finish measuring visibility determination time
    const auto cullingEndTime = std::chrono::high_resolution_clock::now();

    // Convert the culling time into milliseconds
    m_cullingTimeMs = std::chrono::duration<double, std::milli>(cullingEndTime - cullingStartTime).count();

    // --------------------------------------------- Culling ends ---------------------------------------------

    // Get the number of objects that passed the culling test
    const std::size_t visibleObjects = m_visibleObjects.size();

    // Bind shader and send matrices that are shared by every object
    m_triangleShader.Bind();
    m_triangleShader.SetMat4("uView", view);
    m_triangleShader.SetMat4("uProjection", projection);

    // Bind the cube VAO before rendering visible objects
    glBindVertexArray(m_triangleVAO);

    // Render every object that passed the frustum culling test
    for (const SceneObject* object : m_visibleObjects)
    {
        // Build this object's model matrix using its position, rotation and scale
        glm::mat4 model{ 1.0f };

        model = glm::translate(model, object->position);
        model = glm::rotate(model, glm::radians(object->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(object->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(object->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, object->scale);

        // Send the model matrix and colour to the shader for this visible object
        m_triangleShader.SetMat4("uModel", model);
        m_triangleShader.SetVec3("uColour", object->colour);

        // Draw only objects that passed the culling test
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // Unbind the cube VAO after all visible objects have been rendered
    glBindVertexArray(0);

    // Finish measuring total frame time
    const auto frameEndTime = std::chrono::high_resolution_clock::now();

    // Convert the frame time into milliseconds
    m_frameTimeMs = std::chrono::duration<double, std::milli>(frameEndTime - frameStartTime).count();

    // Calculate frames per second from the frame time
    if (m_frameTimeMs > 0.0) m_fps = 1000.0 / m_frameTimeMs;

    // Start a new Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // Draw the real-time performance overlay
    RenderPerformanceOverlay(visibleObjects, totalObjects);

    // Finish building the Dear ImGui frame
    ImGui::Render();

    // Render the Dear ImGui interface over the 3D scene
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update the window title with visible and total object counts
    UpdateWindowTitle(visibleObjects, totalObjects);

    // Present the completed frame to the window
    SDL_GL_SwapWindow(m_window);
}

void Application::Shutdown()
{
    // Shut down the Dear ImGui renderer backend
    ImGui_ImplOpenGL3_Shutdown();

    // Shut down the Dear ImGui SDL3 platform backend
    ImGui_ImplSDL3_Shutdown();

    // Destroy the Dear ImGui context
    ImGui::DestroyContext();

    // Check and delete OpenGL resources
    if (m_triangleVBO != 0)
    {
        glDeleteBuffers
        (
            1,
            &m_triangleVBO
        );

        m_triangleVBO = 0;
    }

    if (m_triangleVAO != 0)
    {
        glDeleteVertexArrays
        (
            1,
            &m_triangleVAO
        );

        m_triangleVAO = 0;
    }

    if (m_glContext != nullptr)
    {
        SDL_GL_DestroyContext(m_glContext);
        m_glContext = nullptr;
    }

    if (m_window != nullptr)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
}

void Application::UpdateWindowTitle(std::size_t visibleObjects, std::size_t totalObjects)
{
    const std::string title = "Visibility Framework | Visible: " + std::to_string(visibleObjects) + " / " + std::to_string(totalObjects);
    SDL_SetWindowTitle( m_window, title.c_str());
}

void Application::RenderPerformanceOverlay(std::size_t visibleObjects, std::size_t totalObjects)
{
    // Calculate how many objects were rejected by visibility determination
    const std::size_t culledObjects = totalObjects - visibleObjects;

    // Convert the current camera mode into readable text
    const char* cameraMode = m_cameraMode == CameraMode::Freecam ? "Freecam" : "Static";

    // Convert the current culling mode into readable text
    const char* cullingMode = m_cullingMode == CullingMode::SingleThreaded ? "Single Thread" : "Multithreaded";

    // Single-threaded mode always reports one active thread
    const std::size_t activeThreads = m_cullingMode == CullingMode::SingleThreaded ? 1 : m_threadCount;

    // Position the performance overlay in the top-left corner
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);

    // Give the overlay a fixed width
    ImGui::SetNextWindowSize(ImVec2(340.0f, 0.0f), ImGuiCond_Always);

    // Keep the performance overlay fixed and unobtrusive
    const ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_AlwaysAutoResize;

    // Begin drawing the performance overlay
    ImGui::Begin("Visibility Determination Framework", nullptr, windowFlags);

    ImGui::Text("Camera:          %s", cameraMode);
    ImGui::Separator();
    ImGui::Text("Objects:         %zu", totalObjects);
    ImGui::Text("Visible:         %zu", visibleObjects);
    ImGui::Text("Culled:          %zu", culledObjects);
    ImGui::Separator();
    ImGui::Text("Culling Mode:    %s", cullingMode);
    ImGui::Text("Threads Used:    %zu", activeThreads);
    ImGui::Text("CPU Max Threads: %zu", m_maxThreadCount);
    ImGui::Text("Culling Time:    %.3f ms", m_cullingTimeMs);
    ImGui::Separator();
    ImGui::Text("Frame Time:      %.3f ms", m_frameTimeMs);
    ImGui::Text("FPS:             %.1f", m_fps);

    ImGui::End();
}

void Application::CycleCullingMode()
{
    // If currently using single-threaded culling, switch to the multithreaded culler starting with two worker threads
    if (m_cullingMode == CullingMode::SingleThreaded)
    {
        // If the CPU cannot support at least two threads, remain in single-threaded mode
        if (m_maxThreadCount < 2) return;

        m_cullingMode = CullingMode::Multithreaded;
        m_threadCount = 2;
        return;
    }

    // Increase the number of worker threads by two
    if (m_threadCount + 2 <= m_maxThreadCount)
    {
        m_threadCount += 2;
        return;
    }

    // If the CPU has an unusual odd maximum thread count, allow the final configuration to use every available thread
    if (m_threadCount < m_maxThreadCount)
    {
        m_threadCount = m_maxThreadCount;
        return;
    }

    // Once the maximum CPU thread count has been reached, wrap back around to the single-threaded implementation
    m_cullingMode = CullingMode::SingleThreaded;
    m_threadCount = 1;
}