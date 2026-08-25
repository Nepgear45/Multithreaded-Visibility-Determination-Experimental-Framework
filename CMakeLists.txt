cmake_minimum_required(VERSION 3.25)

project(
    Multithreaded-Visibility-Determination-Experimental-Framework
    VERSION 1.0
    LANGUAGES CXX
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

# Source files
set(
    SOURCES
    src/main.cpp
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
)
target_link_libraries(
    Multithreaded-Visibility-Determination-Experimental-Framework
    PRIVATE
        SDL3::SDL3
        glm::glm
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

# Windows configuration
if(WIN32)
    target_compile_definitions(
        Multithreaded-Visibility-Determination-Experimental-Framework
        PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
    )

endif()