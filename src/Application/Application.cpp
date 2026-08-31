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
#include <limits>

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

    // Enable VSync
    if (!SDL_GL_SetSwapInterval(1))
    {
        std::cerr << "Warning: VSync could not be enabled: " << SDL_GetError() << '\n';
    }

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


            // F10 - Toggle shortcuts
            if (event.key.scancode == SDL_SCANCODE_F10 && !event.key.repeat) m_showShortcuts = !m_showShortcuts;
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

    // Draw the real-time performance overlay and shortcuts helper
    RenderPerformanceOverlay(visibleObjects, totalObjects);
    RenderShortcutsOverlay();

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
    ImGui::Text("F10  Hide Shortcuts");

    ImGui::End();
}