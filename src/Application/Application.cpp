#include "Application.h"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "System/CPUInfo.h"

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
    glClearColor
    (
        0.1f,
        0.1f,
        0.1f,
        1.0f
    );

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind Shader
    m_triangleShader.Bind();

    // Model Matrix
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::rotate
        (
            model,
            glm::radians(25.0f),
            glm::vec3(1.0f, 0.5f, 0.0f)
        );

    // View Matrix
    glm::mat4 view = glm::lookAt
        (
            glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

    // Projection Matrix
    const float aspectRatio = static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight);

    glm::mat4 projection = glm::perspective
        (
            glm::radians(60.0f),
            aspectRatio,
            0.1f,
            100.0f
        );

    // Send Matrices to Shader
    m_triangleShader.SetMat4
    (
        "uModel",
        model
    );

    m_triangleShader.SetMat4
    (
        "uView",
        view
    );

    m_triangleShader.SetMat4
    (
        "uProjection",
        projection
    );

    // Draw Cube
    glBindVertexArray(m_triangleVAO);
    glDrawArrays
    (
        GL_TRIANGLES,
        0,
        36
    );
    glBindVertexArray(0);

    // Present Frame
    SDL_GL_SwapWindow(m_window);
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