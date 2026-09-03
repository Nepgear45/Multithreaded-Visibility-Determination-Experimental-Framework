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
#include <limits>
#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <sstream>

struct ResolutionPreset
{
    int width;
    int height;
    const char* label;
};

constexpr ResolutionPreset ResolutionPresets[] =
{
    { 1280, 720,  "1280 x 720" },
    { 1600, 900,  "1600 x 900" },
    { 1920, 1080, "1920 x 1080" },
    { 2560, 1440, "2560 x 1440" },
    { 3840, 2160, "3840 x 2160" }
};

namespace
{
    constexpr std::size_t ObjectCountPresets[] =
    {
        100,
        500,
        1000,
        2500,
        5000,
        7500,
        10000
    };

    constexpr std::size_t ObjectCountPresetCount = sizeof(ObjectCountPresets) / sizeof(ObjectCountPresets[0]);
}

constexpr glm::vec3 BenchmarkCameraPosition(0.0f, 15.0f, 35.0f);
constexpr glm::vec3 BenchmarkCameraTarget(0.0f, 0.0f, 0.0f);
constexpr glm::vec3 BenchmarkCameraUp(0.0f, 1.0f, 0.0f);

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

    // Default Window Size
    int windowWidth = 1280;
    int windowHeight = 720;

    // Get Primary Display
    SDL_DisplayID primaryDisplay = SDL_GetPrimaryDisplay();

    if (primaryDisplay != 0)
    {
        SDL_Rect usableBounds;

        if (SDL_GetDisplayUsableBounds(primaryDisplay, &usableBounds))
        {
            std::cout << "\nScreen Size: " << usableBounds.w << " x " << usableBounds.h << '\n';

            if (usableBounds.w >= 3840 && usableBounds.h >= 2160)
            {
                windowWidth = 2560;
                windowHeight = 1440;
            }
            else if (usableBounds.w >= 2560 && usableBounds.h >= 1440)
            {
                windowWidth = 1920;
                windowHeight = 1080;
            }
            else if (usableBounds.w >= 1920 && usableBounds.h >= 1080)
            {
                windowWidth = 1600;
                windowHeight = 900;
            }
            else if (usableBounds.w >= 1600 && usableBounds.h >= 900)
            {
                windowWidth = 1280;
                windowHeight = 720;
            }
            else
            {
                windowWidth = static_cast<int>(usableBounds.w * 0.8f);
                windowHeight = static_cast<int>(usableBounds.h * 0.8f);
            }
        }
        else
        {
            std::cerr << "Failed to get display bounds: " << SDL_GetError() << '\n';
            std::cerr << "Using default window size: 1280 x 720\n";
        }
    }
    else
    {
        std::cerr << "Failed to get primary display: " << SDL_GetError() << '\n';
        std::cerr << "Using default window size: 1280 x 720\n";
    }

    // Create Window
    m_window = SDL_CreateWindow(
        "Multithreaded Visibility Framework",
        windowWidth,
        windowHeight,
        SDL_WINDOW_OPENGL
    );

    if (!m_window)
    {
        std::cerr << "Failed to create SDL window: " << SDL_GetError() << '\n';
        return false;
    }

    // Centre Window
    if (!SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED))
    {
        std::cerr << "Failed to centre window: " << SDL_GetError() << '\n';
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

    // Load OpenGL Functions using GLAD
    int version = gladLoadGL(SDL_GL_GetProcAddress);

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

    // Create the Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForOpenGL(m_window, m_glContext);
    ImGui_ImplOpenGL3_Init("#version 460");    

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
    RegenerateScene();

    // Disable VSync
    if (!SDL_GL_SetSwapInterval(0)) std::cerr << "Warning: VSync could not be disabled: " << SDL_GetError() << '\n';

    // Disable mouse hitting the edge of the window
    SDL_SetWindowRelativeMouseMode(m_window, true);

    // Get the maximum number of concurrent threads reported by the CPU
    m_maxThreadCount = static_cast<std::size_t>(std::thread::hardware_concurrency());

    // Hardware_concurrency can return 0 if the value cannot be determined
    if (m_maxThreadCount == 0) m_maxThreadCount = 2;

    // Create the persistent worker pool using the maximum number of hardware threads reported by the system
    m_persistentMultithreadedCuller.Initialize(m_maxThreadCount);

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
            if (m_cameraMode == CameraMode::Freecam && !m_benchmarkRunning)
            {
                const float xOffset = event.motion.xrel;
                const float yOffset = -event.motion.yrel;

                m_camera.ProcessMouseMovement(xOffset, yOffset);
            }
        }

        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            // F8 - Abort benchmark
            if (event.key.scancode == SDL_SCANCODE_F8 && !event.key.repeat && m_benchmarkRunning) m_benchmarkAbortRequested = true;

            // Lock normal controls while benchmarking
            if (m_benchmarkRunning) continue;

            // F1 - Toggle freecam
            if (event.key.scancode == SDL_SCANCODE_F1 && !event.key.repeat)
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

            // F2 - Cycle through worker thread counts
            if (event.key.scancode == SDL_SCANCODE_F2 && !event.key.repeat) CycleThreadCount();

            // F3 - Cycle through the available culling implementations
            if (event.key.scancode == SDL_SCANCODE_F3 && !event.key.repeat) CycleCullingMode();

            // F4 - Cycle through the object presets
            if (event.key.scancode == SDL_SCANCODE_F4 && !event.key.repeat) CycleObjectCountPreset();

            // F5 - Validate culling results
            if (event.key.scancode == SDL_SCANCODE_F5 && !event.key.repeat) m_validationRequested = true;

            // F6 - Cycle resolution
            if (event.key.scancode == SDL_SCANCODE_F6 && !event.key.repeat) CycleResolution();

            // F7 - Start benchmark
            if (event.key.scancode == SDL_SCANCODE_F7 && !event.key.repeat) StartBenchmark();

            // F10 - Toggle shortcuts
            if (event.key.scancode == SDL_SCANCODE_F10 && !event.key.repeat) m_showShortcuts = !m_showShortcuts;
        }
    }
}

void Application::Update()
{
    if (m_cameraMode != CameraMode::Freecam) return;
    if (m_benchmarkRunning) return;

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
    // Process benchmark request cancel
    if (m_benchmarkAbortRequested) StopBenchmark(false);

    // Start measuring total frame time
    const auto frameStartTime = Clock::now();

    // Get Object Count
    const std::size_t totalObjects = m_scene.GetObjects().size();

    // Clear Buffers
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate camera matrices
    // Calculate camera matrices
    const glm::mat4 view = m_benchmarkRunning ? glm::lookAt(BenchmarkCameraPosition, BenchmarkCameraTarget, BenchmarkCameraUp) : m_camera.GetViewMatrix();
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
    const auto cullingStartTime = Clock::now();

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
    else if (m_cullingMode == CullingMode::Multithreaded)
    {
        m_multithreadedCuller.Cull
        (
            m_scene.GetObjects(),
            frustum,
            m_visibleObjects,
            m_threadCount
        );
    }
    else
    {
        m_persistentMultithreadedCuller.Cull
        (
            m_scene.GetObjects(),
            frustum,
            m_visibleObjects,
            m_threadCount
        );
}

    // Finish measuring visibility determination time
    const auto cullingEndTime = Clock::now();

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
    const auto frameEndTime = Clock::now();

    // Convert the frame time into milliseconds
    m_frameTimeMs = std::chrono::duration<double, std::milli>(frameEndTime - frameStartTime).count();

    // Calculate frames per second from the frame time
    if (m_frameTimeMs > 0.0) m_fps = 1000.0 / m_frameTimeMs;

    // Benchmark run
    if (m_benchmarkRunning) UpdateBenchmark(frustum, visibleObjects);

    // Run requested culling validation outside normal performance measurements
    if (m_validationRequested)
    {
        ValidateCullingResults(frustum);
        m_validationRequested = false;
    }

    // Start a new Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // Draw the real-time performance overlay and shortcuts helper
    if (m_benchmarkRunning) RenderBenchmarkUI();
    else
    {
        RenderPerformanceOverlay(visibleObjects, totalObjects);
        RenderShortcutsOverlay();
    }

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

    // Single-threaded mode always reports one active thread
    const std::size_t activeThreads = m_cullingMode == CullingMode::SingleThreaded ? 1 : m_threadCount;

    // Position the performance overlay in the top-left corner
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);

    // Give the overlay a fixed width
    ImGui::SetNextWindowSize(ImVec2(450.0f, 0.0f), ImGuiCond_Always);

    // Keep the performance overlay fixed and unobtrusive
    const ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_AlwaysAutoResize;

    // Begin drawing the performance overlay
    ImGui::Begin("Visibility Determination Framework", nullptr, windowFlags);

    ImGui::Text("Culling Configuration");
    ImGui::Separator();

    const char* cullingModeLabels[] =
    {
        "Single Thread",
        "Basic Multithreaded",
        "Persistent Multithreaded"
    };

    int selectedCullingMode = static_cast<int>(m_cullingMode);

    ImGui::Text("Implementation:");
    ImGui::SameLine(180.0f);
    ImGui::SetNextItemWidth(200.0f);
    
    if (ImGui::Combo("##CullingMode", &selectedCullingMode, cullingModeLabels, IM_ARRAYSIZE(cullingModeLabels))) m_cullingMode = static_cast<CullingMode>(selectedCullingMode);

    // Display the active thread count
    ImGui::Text("Worker Threads: %zu", activeThreads);

    // Only show the thread slider for multithreaded implementations
    if (m_cullingMode != CullingMode::SingleThreaded)
    {
        ImGui::SameLine();

        // Convert the worker count into a slider position
        int threadStep = static_cast<int>(m_threadCount / 2);
        const int maximumThreadStep = static_cast<int>(m_maxThreadCount / 2);

        // Set the width of the thread-count slider
        ImGui::SetNextItemWidth(160.0f);

        // Convert the slider position back into a worker count
        if (ImGui::SliderInt("##ThreadCount", &threadStep, 1, maximumThreadStep, "")) m_threadCount = static_cast<std::size_t>(threadStep * 2);
    }

    ImGui::Text("Maximum Threads: %zu", m_maxThreadCount);

    ImGui::Spacing();

    ImGui::Text("Scene Configuration");
    ImGui::Separator();

    // Store Previous Unlock State
    const bool previousUnlockedState = m_unlockedObjectCount;

    // Unlock Object Count
    ImGui::Checkbox("Unlock Object Count", &m_unlockedObjectCount);

    // Snap Back To Closest Preset
    if (previousUnlockedState && !m_unlockedObjectCount)
    {
        std::size_t closestPresetIndex = 0;
        std::size_t smallestDifference = std::numeric_limits<std::size_t>::max();

        for (std::size_t i = 0; i < std::size(ObjectCountPresets); ++i)
        {
            const std::size_t presetValue = ObjectCountPresets[i];
            const std::size_t difference = presetValue > m_requestedObjectCount ? presetValue - m_requestedObjectCount : m_requestedObjectCount - presetValue;

            if (difference < smallestDifference)
            {
                smallestDifference = difference;
                closestPresetIndex = i;
            }
        }

        m_objectCountPresetIndex = closestPresetIndex;
        m_requestedObjectCount = ObjectCountPresets[m_objectCountPresetIndex];

        RegenerateScene();
    }

    ImGui::Spacing();

    // Preset Object Count
    if (!m_unlockedObjectCount)
    {
        const char* objectCountPresetLabels[] =
        {
            "100",
            "500",
            "1,000",
            "2,500",
            "5,000",
            "7,500",
            "10,000"
        };

        int selectedPreset = static_cast<int>(m_objectCountPresetIndex);

        ImGui::Text("Object Count: %zu", m_requestedObjectCount);
        ImGui::SameLine(180.0f);
        ImGui::SetNextItemWidth(200.0f);

        if (ImGui::Combo("##ObjectCountPreset", &selectedPreset, objectCountPresetLabels, IM_ARRAYSIZE(objectCountPresetLabels)))
        {
            m_objectCountPresetIndex = static_cast<std::size_t>(selectedPreset);
            m_requestedObjectCount = ObjectCountPresets[m_objectCountPresetIndex];
            RegenerateScene();
        }
    }
    // Unlocked Object Count
    else
    {
        int objectCountStep = static_cast<int>(m_requestedObjectCount / 100);

        ImGui::Text("Object Count: %zu", m_requestedObjectCount);
        ImGui::SameLine(180.0f);
        ImGui::SetNextItemWidth(200.0f);

        if (ImGui::SliderInt("##ObjectCount", &objectCountStep, 1, 100, ""))
        {
            m_requestedObjectCount = static_cast<std::size_t>(objectCountStep * 100);
            RegenerateScene();
        }
    }

    ImGui::Spacing();

    ImGui::Text("Culling Statistics");
    ImGui::Separator();

    ImGui::Text("Objects:       %zu", totalObjects);
    ImGui::Text("Visible:       %zu", visibleObjects);
    ImGui::Text("Culled:        %zu", culledObjects);

    ImGui::Spacing();

    ImGui::Text("Culling Validation");
    ImGui::Separator();

    if (!m_hasValidationResult) ImGui::Text("Last Result:    Not Run");
    else
    {
        ImGui::Text("Last Result:    %s", m_cullingResultsMatch ? "Match" : "MISMATCH");
        ImGui::Text("Objects Tested: %zu", m_validationObjectCount);
        ImGui::Text("Single Thread:  %zu", m_validationSingleVisible);
        ImGui::Text("Basic MT:       %zu", m_validationBasicVisible);
        ImGui::Text("Persistent MT:  %zu", m_validationPersistentVisible);
    }

    ImGui::Spacing();

    ImGui::Text("Benchmark");
    ImGui::Separator();

    if (!m_benchmarkRunning)
    {
        if (ImGui::Button("Start Benchmark")) StartBenchmark();
    }
    else
    {
        ImGui::Text("Status: Running");
        if (ImGui::Button("Abort Benchmark")) m_benchmarkAbortRequested = true;
    }

    ImGui::Spacing();

    ImGui::Text("Display Configuration");
    ImGui::Separator();

    const char* resolutionLabels[] =
    {
        "1280 x 720",
        "1600 x 900",
        "1920 x 1080",
        "2560 x 1440",
        "3840 x 2160"
    };

    ImGui::Text("Resolution:");
    ImGui::SameLine(180.0f);
    ImGui::SetNextItemWidth(200.0f);

    if (ImGui::Combo("##Resolution", &m_resolutionIndex, resolutionLabels, IM_ARRAYSIZE(resolutionLabels)))
    {
        const ResolutionPreset& resolution = ResolutionPresets[m_resolutionIndex];
        SetWindowResolution(resolution.width, resolution.height);
    }

    ImGui::Spacing();

    ImGui::Text("Performance");
    ImGui::Separator();

    ImGui::Text("Culling Time:  %.3f ms", m_cullingTimeMs);
    ImGui::Text("Frame Time:    %.3f ms", m_frameTimeMs);
    ImGui::Text("FPS:           %.1f", m_fps);

    ImGui::Spacing();
    ImGui::TextDisabled("Press F10 to show shortcuts");

    ImGui::End();
}

void Application::CycleThreadCount()
{
    // Start at two worker threads
    if (m_threadCount < 2)
    {
        m_threadCount = 2;
        return;
    }

    // Increase the worker count by two while staying within the maximum supported hardware thread count
    if (m_threadCount + 2 <= m_maxThreadCount)
    {
        m_threadCount += 2;
        return;
    }

    // If the maximum hardware thread count is odd, allow the final configuration to use that value
    if (m_threadCount < m_maxThreadCount)
    {
        m_threadCount = m_maxThreadCount;
        return;
    }

    // Wrap back to two worker threads
    m_threadCount = 2;
}

void Application::CycleCullingMode()
{
    // Change culling mode
    if (m_cullingMode == CullingMode::SingleThreaded) m_cullingMode = CullingMode::Multithreaded;
    else if (m_cullingMode == CullingMode::Multithreaded) m_cullingMode = CullingMode::PersistentMultithreaded;
    else m_cullingMode = CullingMode::SingleThreaded;
}

void Application::RegenerateScene()
{
    m_scene.GenerateGrid(m_requestedObjectCount, 2.0f);
}

void Application::CycleObjectCountPreset()
{
    if (m_unlockedObjectCount) return;

    ++m_objectCountPresetIndex;

    if (m_objectCountPresetIndex >= std::size(ObjectCountPresets)) m_objectCountPresetIndex = 0;

    m_requestedObjectCount = ObjectCountPresets[m_objectCountPresetIndex];

    RegenerateScene();
}

void Application::RenderShortcutsOverlay()
{
    if (!m_showShortcuts) return;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 10.0f, viewport->WorkPos.y + 10.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));

    const ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("Shortcuts", nullptr, windowFlags);

    ImGui::Text("F1   Camera Mode");
    ImGui::Text("F2   Cycle Worker Threads");
    ImGui::Text("F3   Cycle Culling Implementation");
    ImGui::Text("F4   Cycle Object Count Preset");
    ImGui::Text("F5   Validate Culling Results");
    ImGui::Text("F6   Cycle Resolution");
    ImGui::Text("F7   Start Benchmark");
    ImGui::Text("F8   Abort Benchmark");

    ImGui::Text("F10  Hide Shortcuts");

    ImGui::End();
}

bool Application::ValidateCullingResults(const Frustum& frustum)
{
    std::vector<const SceneObject*> singleThreadedResults;
    std::vector<const SceneObject*> basicMultithreadedResults;
    std::vector<const SceneObject*> persistentMultithreadedResults;

    // Run all culling implementations
    m_singleThreadedCuller.Cull(m_scene.GetObjects(), frustum, singleThreadedResults);
    m_multithreadedCuller.Cull(m_scene.GetObjects(), frustum, basicMultithreadedResults, m_threadCount);
    m_persistentMultithreadedCuller.Cull(m_scene.GetObjects(), frustum, persistentMultithreadedResults, m_threadCount);

    // Sort results so execution order does not affect comparison
    std::sort(singleThreadedResults.begin(), singleThreadedResults.end());
    std::sort(basicMultithreadedResults.begin(), basicMultithreadedResults.end());
    std::sort(persistentMultithreadedResults.begin(), persistentMultithreadedResults.end());

    // Store diagnostic information
    m_validationSingleVisible = singleThreadedResults.size();
    m_validationBasicVisible = basicMultithreadedResults.size();
    m_validationPersistentVisible = persistentMultithreadedResults.size();
    m_validationObjectCount = m_scene.GetObjects().size();

    // All implementations must return exactly the same objects
    m_cullingResultsMatch = singleThreadedResults == basicMultithreadedResults && singleThreadedResults == persistentMultithreadedResults;
    m_hasValidationResult = true;

    if (m_cullingResultsMatch) std::cout << "Culling validation passed: all implementations match.\n";
    else
    {
        std::cout << "Culling validation failed.\n";
        std::cout << "Single Thread: " << m_validationSingleVisible << '\n';
        std::cout << "Basic Multithreaded: " << m_validationBasicVisible << '\n';
        std::cout << "Persistent Multithreaded: " << m_validationPersistentVisible << '\n';
    }

    return m_cullingResultsMatch;
}

void Application::SetWindowResolution(int width, int height)
{
    m_windowWidth = width;
    m_windowHeight = height;
    SDL_SetWindowSize(m_window, m_windowWidth, m_windowHeight);
    glViewport(0, 0, m_windowWidth, m_windowHeight);
}

void Application::CycleResolution()
{
    m_resolutionIndex++;
    if (m_resolutionIndex >= static_cast<int>(std::size(ResolutionPresets))) m_resolutionIndex = 0; // std::size is comfy

    const ResolutionPreset& resolution = ResolutionPresets[m_resolutionIndex];
    SetWindowResolution(resolution.width, resolution.height);
}

void Application::StopBenchmark(bool completed)
{
    if (!m_benchmarkRunning) return;

    m_benchmarkRunning = false;
    m_benchmarkAbortRequested = false;

    if (m_benchmarkOutputFile.is_open()) m_benchmarkOutputFile.close();    
    if (!m_benchmarkOutputPath.empty()) std::cout << "Benchmark output: " << m_benchmarkOutputPath << '\n';

    std::cout << (completed ? "Benchmark completed.\n" : "Benchmark aborted.\n");

    if (m_cameraMode == CameraMode::Freecam) SDL_SetWindowRelativeMouseMode(m_window, true);
}

void Application::RenderBenchmarkUI()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 10.0f, viewport->WorkPos.y + 10.0f), ImGuiCond_Always);

    ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f));

    const ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings;

    const char* benchmarkPhase = "Unknown";

    if (m_benchmarkPhase == BenchmarkPhase::Validation) benchmarkPhase = "Validation";
    else if (m_benchmarkPhase == BenchmarkPhase::Warmup) benchmarkPhase = "Warm-up";
    else if (m_benchmarkPhase == BenchmarkPhase::Measurement) benchmarkPhase = "Measurement";

    ImGui::Begin("Benchmark", nullptr, windowFlags);

    ImGui::Text("Status: Running");

    ImGui::Spacing();
    ImGui::Separator();

    const BenchmarkConfiguration& configuration = m_benchmarkConfigurations[m_currentBenchmarkTest];

    const char* cullingModeLabels[] =
    {
        "Single Thread",
        "Basic Multithreaded",
        "Persistent Multithreaded"
    };

    ImGui::Text("Phase:  %s", benchmarkPhase);
    ImGui::Text("Test:            %zu / %zu", m_currentBenchmarkTest + 1, m_benchmarkConfigurations.size());
    ImGui::Text("Implementation:  %s", cullingModeLabels[static_cast<int>(configuration.cullingMode)]);
    ImGui::Text("Objects:         %zu", configuration.objectCount);
    ImGui::Text("Worker Threads:  %zu", configuration.threadCount);
    if (m_benchmarkPhase == BenchmarkPhase::Warmup) ImGui::Text("Warm-up Frame:   %zu / %zu", m_currentWarmupFrame, m_warmupFramesPerBenchmarkTest);
    else if (m_benchmarkPhase == BenchmarkPhase::Measurement) ImGui::Text("Sample:          %zu / %zu", m_currentBenchmarkSample, m_samplesPerBenchmarkTest);

    ImGui::Spacing();

    ImGui::Text("Culling Time:    %.3f ms", m_cullingTimeMs);
    ImGui::Text("Frame Time:      %.3f ms", m_frameTimeMs);
    ImGui::Text("FPS:             %.1f", m_fps);

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::TextDisabled("Press F8 to abort benchmark");

    ImGui::End();
}

void Application::BuildBenchmarkConfigurations()
{
    m_benchmarkConfigurations.clear();

    constexpr std::size_t objectCounts[] =
    {
        100,
        500,
        1000,
        2500,
        5000,
        7500,
        10000
    };

    constexpr std::size_t threadCounts[] =
    {
        2,
        4,
        8,
        16,
        32
    };

    /*
    constexpr std::size_t objectCounts[] =
    {
        300
    };

    constexpr std::size_t threadCounts[] =
    {
        2
    };
    */

    for (const std::size_t objectCount : objectCounts)
    {
        // Single-threaded baseline
        m_benchmarkConfigurations.push_back({CullingMode::SingleThreaded, objectCount, 1});

        // Basic multithreaded
        for (const std::size_t threadCount : threadCounts)
        {
            m_benchmarkConfigurations.push_back({CullingMode::Multithreaded, objectCount, threadCount});
        }

        // Persistent multithreaded
        for (const std::size_t threadCount : threadCounts)
        {
            m_benchmarkConfigurations.push_back({CullingMode::PersistentMultithreaded, objectCount, threadCount});
        }
    }
}

void Application::StartBenchmark()
{
    if (m_benchmarkRunning) return;

    BuildBenchmarkConfigurations();

    if (m_benchmarkConfigurations.empty()) return;

    if (!OpenBenchmarkOutputFile()) return;

    m_benchmarkRunning = true;
    m_benchmarkAbortRequested = false;

    SDL_SetWindowRelativeMouseMode(m_window, false);

    m_currentBenchmarkTest = 0;
    m_currentBenchmarkSample = 0;

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "Benchmark Run\n";
    std::cout << "Run ID:          " << m_benchmarkRunId << '\n';
    std::cout << "Warm-up Frames:  " << m_warmupFramesPerBenchmarkTest << '\n';
    std::cout << "Samples/Test:    " << m_samplesPerBenchmarkTest << '\n';
    std::cout << "Configurations:  " << m_benchmarkConfigurations.size() << '\n';
    std::cout << "========================================\n";

    ApplyBenchmarkConfiguration();

    std::cout << "Benchmark started.\n";
}

void Application::ApplyBenchmarkConfiguration()
{
    if (m_currentBenchmarkTest >= m_benchmarkConfigurations.size()) return;

    const BenchmarkConfiguration& configuration = m_benchmarkConfigurations[m_currentBenchmarkTest];

    m_cullingMode = configuration.cullingMode;
    m_threadCount = configuration.threadCount;
    m_requestedObjectCount = configuration.objectCount;

    m_resolutionIndex = 2;
    SetWindowResolution(1920, 1080);

    RegenerateScene();

    // Reset benchmark measurements
    m_currentBenchmarkSample = 0;
    m_currentWarmupFrame = 0;
    m_benchmarkSamples.clear();

    // Using .reserve because it is predetermined
    m_benchmarkSamples.reserve(m_samplesPerBenchmarkTest);

    m_benchmarkPhase = BenchmarkPhase::Validation;

    PrintBenchmarkConfiguration();
}

void Application::PrintBenchmarkConfiguration() const
{
    const BenchmarkConfiguration& configuration =
        m_benchmarkConfigurations[m_currentBenchmarkTest];

    const char* cullingModeLabels[] =
    {
        "Single Thread",
        "Basic Multithreaded",
        "Persistent Multithreaded"
    };

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "Benchmark Test " << m_currentBenchmarkTest + 1 << " / " << m_benchmarkConfigurations.size() << '\n';
    std::cout << "Implementation: " << cullingModeLabels[static_cast<int>(configuration.cullingMode)] << '\n';
    std::cout << "Objects:        " << configuration.objectCount << '\n';
    std::cout << "Worker Threads: " << configuration.threadCount << '\n';
    std::cout << "========================================\n";
}

void Application::PrintBenchmarkResults() const
{
    if (m_benchmarkSamples.empty()) return;

    double cullingTimeTotal = 0.0;
    double frameTimeTotal = 0.0;

    for (const BenchmarkSample& sample : m_benchmarkSamples)
    {
        cullingTimeTotal += sample.cullingTimeMs;
        frameTimeTotal += sample.frameTimeMs;
    }

    const double averageCullingTime = cullingTimeTotal / static_cast<double>(m_benchmarkSamples.size());
    const double averageFrameTime = frameTimeTotal / static_cast<double>(m_benchmarkSamples.size());
    const double averageFPS = averageFrameTime > 0.0 ? 1000.0 / averageFrameTime : 0.0;

    const std::size_t expectedVisibleCount = m_benchmarkSamples.front().visibleObjectCount;

    bool visibleCountStable = true;

    for (const BenchmarkSample& sample : m_benchmarkSamples)
    {
        if (sample.visibleObjectCount != expectedVisibleCount)
        {
            visibleCountStable = false;
            break;
        }
    }

    std::cout << "========================================\n";
    std::cout << "Test Complete\n";
    std::cout << "Samples:              " << m_benchmarkSamples.size() << '\n';
    std::cout << "First Sample:         " << m_benchmarkSamples.front().sampleIndex << '\n';
    std::cout << "Last Sample:          " << m_benchmarkSamples.back().sampleIndex << '\n';
    std::cout << "Average Culling Time: " << averageCullingTime << " ms\n";
    std::cout << "Average Frame Time:   " << averageFrameTime << " ms\n";
    std::cout << "Average FPS:          " << averageFPS << '\n';
    std::cout << "Visible Objects:      " << expectedVisibleCount << '\n';
    std::cout << "Visibility Stable:    " << (visibleCountStable ? "Yes" : "NO") << '\n';
    std::cout << "========================================\n";
}

void Application::UpdateBenchmark(const Frustum& frustum, std::size_t visibleObjectCount)
{
    if (!m_benchmarkRunning) return;

    if (m_benchmarkAbortRequested)
    {
        StopBenchmark(false);
        return;
    }

    // Validate the current configuration before measuring it
    if (m_benchmarkPhase == BenchmarkPhase::Validation)
    {
        if (!ValidateCullingResults(frustum))
        {
            std::cout << "Benchmark aborted due to culling validation failure.\n";
            StopBenchmark(false);
            return;
        }

        m_benchmarkPhase = BenchmarkPhase::Warmup;

        std::cout << "Starting warm-up.\n";
        return;
    }

    // Allow the current configuration to settle before measurements
    if (m_benchmarkPhase == BenchmarkPhase::Warmup)
    {
        m_currentWarmupFrame++;

        if (m_currentWarmupFrame >= m_warmupFramesPerBenchmarkTest)
        {
            m_benchmarkPhase = BenchmarkPhase::Measurement;
            std::cout << "Warm-up complete. Starting measurement.\n";
        }

        return;
    }

    // Record the current frame
    if (m_benchmarkPhase == BenchmarkPhase::Measurement)
    {
        BenchmarkSample sample;
        sample.sampleIndex = m_currentBenchmarkSample + 1;
        sample.cullingTimeMs = m_cullingTimeMs;
        sample.frameTimeMs = m_frameTimeMs;
        sample.visibleObjectCount = visibleObjectCount;

        m_benchmarkSamples.push_back(sample);
        m_currentBenchmarkSample++;

        if (m_currentBenchmarkSample >= m_samplesPerBenchmarkTest)
        {
            PrintBenchmarkResults();
            WriteBenchmarkSamples();

            m_currentBenchmarkTest++;

            // Finish after the final configuration
            if (m_currentBenchmarkTest >= m_benchmarkConfigurations.size())
            {
                std::cout << "\nBenchmark completed successfully.\n";
                StopBenchmark(true);
                return;
            }

            ApplyBenchmarkConfiguration();
        }

        return;
    }
}

bool Application::OpenBenchmarkOutputFile()
{
    const std::filesystem::path outputDirectory = "results/raw";

    std::filesystem::create_directories(outputDirectory);

    const auto now = std::chrono::system_clock::now();
    const std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};

#ifdef _WIN32
    localtime_s(&localTime, &currentTime);
#else
    localtime_r(&currentTime, &localTime);
#endif

    std::ostringstream runId;
    runId << std::put_time(&localTime, "%Y-%m-%d_%H-%M-%S");
    m_benchmarkRunId = runId.str();
    m_benchmarkOutputPath = outputDirectory / ("benchmark_" + m_benchmarkRunId + ".csv");
    m_benchmarkOutputFile.open(m_benchmarkOutputPath);

    if (!m_benchmarkOutputFile.is_open())
    {
        std::cerr << "Failed to create benchmark output file: " << m_benchmarkOutputPath << '\n';
        return false;
    }

    m_benchmarkOutputFile
        << "test_index,"
        << "sample_index,"
        << "implementation,"
        << "object_count,"
        << "worker_threads,"
        << "visible_objects,"
        << "culling_time_ms,"
        << "frame_time_ms\n";

    return true;
}

void Application::WriteBenchmarkSamples()
{
    if (!m_benchmarkOutputFile.is_open()) return;
    if (m_currentBenchmarkTest >= m_benchmarkConfigurations.size()) return;

    const BenchmarkConfiguration& configuration = m_benchmarkConfigurations[m_currentBenchmarkTest];

    const char* implementation = "Unknown";
    
    if (configuration.cullingMode == CullingMode::SingleThreaded) implementation = "SingleThreaded";
    else if (configuration.cullingMode == CullingMode::Multithreaded) implementation = "BasicMultithreaded";
    else if (configuration.cullingMode == CullingMode::PersistentMultithreaded) implementation = "PersistentMultithreaded";

    m_benchmarkOutputFile << std::fixed << std::setprecision(6);

    for (const BenchmarkSample& sample : m_benchmarkSamples)
    {
        m_benchmarkOutputFile
            << m_currentBenchmarkTest + 1 << ','
            << sample.sampleIndex << ','
            << implementation << ','
            << configuration.objectCount << ','
            << configuration.threadCount << ','
            << sample.visibleObjectCount << ','
            << sample.cullingTimeMs << ','
            << sample.frameTimeMs << '\n';
    }

    m_benchmarkOutputFile.flush();
}