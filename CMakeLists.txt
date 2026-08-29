cmake_minimum_required(VERSION 3.25)

project(
    Multithreaded-Visibility-Determination-Experimental-Framework
    VERSION 1.0
    LANGUAGES C CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Visual Studio settings
if(MSVC)
    add_compile_options(/W4)
endif()

# Dependencies
include(FetchContent)

# SDL3
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.16
)
FetchContent_MakeAvailable(SDL3)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# Tracy Profiler
FetchContent_Declare(
    Tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy.git
    GIT_TAG v0.11.1
)
FetchContent_MakeAvailable(Tracy)

# GLAD
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.8
    SOURCE_SUBDIR cmake
)
FetchContent_MakeAvailable(glad)

glad_add_library(
    glad_gl
    STATIC
    REPRODUCIBLE
    LOADER
    API gl:core=4.6
)

# Source files
set(
    SOURCES

    src/main.cpp

    src/System/CPUInfo.h
    src/System/CPUInfo.cpp

    src/Camera/Camera.h
    src/Camera/Camera.cpp

    src/Scene/SceneObject.h
    src/Scene/Scene.h
    src/Scene/Scene.cpp

    src/Visibility/AABB.h
    src/Visibility/AABB.cpp
    src/Visibility/Frustum.h
    src/Visibility/Frustum.cpp

    src/Visibility/SingleThreadedCuller.h
    src/Visibility/SingleThreadedCuller.cpp
    src/Visibility/MultithreadedCuller.h
    src/Visibility/MultithreadedCuller.cpp

    src/Application/Application.h
    src/Application/Application.cpp
    src/Rendering/Shader.cpp
    src/Rendering/Shader.cpp    
)

# Executable
add_executable(
    Multithreaded-Visibility-Determination-Experimental-Framework
    ${SOURCES}
)

# Include directories
target_include_directories(
    Multithreaded-Visibility-Determination-Experimental-Framework
    PRIVATE

        ${CMAKE_CURRENT_SOURCE_DIR}/src

        ${CMAKE_CURRENT_BINARY_DIR}/gladsources/glad_gl/include
)

# Libraries
target_link_libraries(
    Multithreaded-Visibility-Determination-Experimental-Framework
    PRIVATE

        SDL3::SDL3
        glm::glm
        glad_gl
)

# Tracy
target_compile_definitions(
    Multithreaded-Visibility-Determination-Experimental-Framework
    PRIVATE

        TRACY_ENABLE
)

target_link_libraries(
    Multithreaded-Visibility-Determination-Experimental-Framework
    PRIVATE

        TracyClient
)

# Copy SDL3 runtime DLL
if(WIN32)
    add_custom_command(
        TARGET Multithreaded-Visibility-Determination-Experimental-Framework
        POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:SDL3::SDL3>"
            "$<TARGET_FILE_DIR:Multithreaded-Visibility-Determination-Experimental-Framework>"
    )
endif()

# Copy Shaders
add_custom_command(
    TARGET Multithreaded-Visibility-Determination-Experimental-Framework
    POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:Multithreaded-Visibility-Determination-Experimental-Framework>/shaders"
)    

# Windows configuration
if(WIN32)
    target_compile_definitions(
        Multithreaded-Visibility-Determination-Experimental-Framework
        PRIVATE

            WIN32_LEAN_AND_MEAN
            NOMINMAX
    )
endif()