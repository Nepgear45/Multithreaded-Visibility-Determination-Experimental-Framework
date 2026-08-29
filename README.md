# Multithreaded Visibility Determination Experimental Framework
An experimental real-time rendering framework developed to investigate the performance impact of **multithreaded CPU-side visibility determination**.
The framework is being developed as part of a research project comparing single-threaded and multithreaded visibility determination techniques under different scene and threading configurations.

## Current Features
* SDL3 window and input handling
* OpenGL 4.6 Core rendering
* GLAD OpenGL function loading
* GLM mathematics
* Tracy profiling integration
* Basic shader compilation and linking
* 3D rendering with model, view and projection matrices
* Depth testing
* CPU hardware and topology information
* Cross-platform CPU information implementation for Windows and Linux (Linux implementation is strictly experimental)
* Camera system
* Freecam movement and mouse look
* Static camera mode

## Camera Controls
| Input       | Action                                         |
| ----------- | ---------------------------------------------- |
| `W`         | Move forward                                   |
| `S`         | Move backward                                  |
| `A`         | Move left                                      |
| `D`         | Move right                                     |
| `Space`     | Move up                                        |
| `Left Ctrl` | Move down                                      |
| `Mouse`     | Rotate / look around                           |
| `F1`        | Toggle between Freecam and Static camera modes |

### Freecam Mode
Freecam allows the camera to move freely through the rendered scene using the keyboard and mouse.
When Freecam is enabled, relative mouse mode is enabled so that the camera can rotate continuously without the cursor being constrained by the edges of the application window.

Note that Freecam has pitch limit of +-89 degrees to stop the camera from fliping over. Yaw is not unlimited.

### Static Mode
Static mode disables Freecam movement and mouse rotation.
Press `F1` to switch between Static and Freecam modes.
Static mode will be used for benchamrking.

## Technologies Used
* x86-64 operating system architecure
* C++20
* Python 3.13
* CMake
* SDL3
* OpenGL 4.6 Core
* GLAD 2.0.8
* GLM
* Tracy Profiler

## Build
The project uses CMake.

### Configure
From the project root:

```powershell
cmake -S . -B build
```

Python 3.13.x is specifically required for installation of GLAD 2.0.8:

```powershell
cmake -S . -B build -DPython_EXECUTABLE="path/to/python.exe"
```

This explicitly selects the Python 3.13 interpreter used by GLAD during OpenGL source generation.

### Build Debug
```powershell
cmake --build build --config Debug
```

### Build Release
```powershell
cmake --build build --config Release
```
