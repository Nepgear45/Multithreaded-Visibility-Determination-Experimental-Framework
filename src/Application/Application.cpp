#include "Application.h"

#include <glad/gl.h>

#include <windows.h>

#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>
#include <unordered_set>
#include <string>

Application::Application()
{

}

Application::~Application()
{
    Shutdown();
}

static void PrintCPUInformation()
{
    // CPU Retail Name
    std::string retailName = "Unknown";
    HKEY key;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &key) == ERROR_SUCCESS)
    {
        char buffer[256]{};
        DWORD bufferSize = sizeof(buffer);

        if (RegQueryValueExA(key, "ProcessorNameString", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufferSize) == ERROR_SUCCESS)
        {
            retailName = buffer;
        }

        RegCloseKey(key);
    }

    // CPU Model
    const char* cpuModel = std::getenv("PROCESSOR_IDENTIFIER");

    // CPU Topology
    DWORD length = 0;

    GetSystemCpuSetInformation
    (
        nullptr,
        0,
        &length,
        GetCurrentProcess(),
        0
    );

    std::vector<unsigned char> cpuBuffer(length);

    if (!GetSystemCpuSetInformation(reinterpret_cast<PSYSTEM_CPU_SET_INFORMATION>(cpuBuffer.data()), length, &length, GetCurrentProcess(), 0))
    {
        std::cerr << "Failed to retrieve CPU information.\n";
        return;
    }

    std::unordered_set<ULONG> physicalCores;
    std::unordered_set<ULONG> performanceCores;
    std::unordered_set<ULONG> efficiencyCores;

    DWORD offset = 0;

    while (offset < length)
    {
        auto* info = reinterpret_cast<PSYSTEM_CPU_SET_INFORMATION>(cpuBuffer.data() + offset);

        if (info->Type == CpuSetInformation)
        {
            const auto& cpu = info->CpuSet;

            physicalCores.insert(cpu.CoreIndex);

            // Note
            // EfficiencyClass 0 represents the
            // highest-performance core class.

            if (cpu.EfficiencyClass == 0)
            {
                efficiencyCores.insert(cpu.CoreIndex);
            }
            else
            {
                performanceCores.insert(cpu.CoreIndex);
            }
        }

        offset += info->Size;
    }

    // Output
    std::cout << "\n--- CPU Information ---\n";
    std::cout << "Retail Name: " << retailName << '\n';
    std::cout << "Model: " << (cpuModel != nullptr ? cpuModel : "Unknown") << '\n';
    std::cout << "Physical Cores: " << physicalCores.size() << '\n';
    std::cout << "Performance Cores (P-Cores): " << performanceCores.size() << '\n';
    std::cout << "Efficiency Cores (E-Cores): " << efficiencyCores.size() << '\n';
    std::cout << "Logical Processors: " << std::thread::hardware_concurrency() << '\n';
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
    PrintCPUInformation();

    // Display OpenGL Information
    std::cout << "\n--- OpenGL Information ---\n";
    std::cout << "GLAD loaded OpenGL " << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << '\n';
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << '\n';
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << '\n';
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << '\n';
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';
    std::cout << "OpenGL " << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << " loaded successfully." << '\n';

    // Configure Viewport
    glViewport
    (
        0,
        0,
        m_windowWidth,
        m_windowHeight
    );

    // Load Triangle Shader
    if (!m_triangleShader.LoadFromFiles("shaders/triangle.vert", "shaders/triangle.frag"))
    {
        std::cerr << "Failed to load triangle shader." << '\n';
        return false;
    }

    // Triangle Geometry
    const float vertices[] =
    {
         0.0f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f
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

    // Enable VSync
    if (!SDL_GL_SetSwapInterval(1))
    {
        std::cerr << "Warning: VSync could not be enabled: " << SDL_GetError() << '\n';
    }

    // Application State
    m_running = true;
    std::cout << '\n' << "Application initialised successfully." << '\n';

    return true;
}

void Application::Run()
{
    while (m_running)
    {
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
        if (event.type == SDL_EVENT_QUIT)
        {
            m_running = false;
        }
    }
}

void Application::Update()
{

}

void Application::Render()
{
    // Clear
    glClearColor
    (
        0.1f,
        0.1f,
        0.1f,
        1.0f
    );

    glClear
    (
        GL_COLOR_BUFFER_BIT
    );

    // Draw Triangle
    m_triangleShader.Bind();

    glBindVertexArray
    (
        m_triangleVAO
    );

    glDrawArrays
    (
        GL_TRIANGLES,
        0,
        3
    );

    glBindVertexArray(0);

    // Present Frame
    SDL_GL_SwapWindow
    (
        m_window
    );
}

void Application::Shutdown()
{
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