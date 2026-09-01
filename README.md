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
* Deterministic scene generation
* Configurable scene object counts and presets
* Per-object axis-aligned bounding boxes (AABBs)
* World-space AABB transformation
* Camera frustum extraction
* CPU-side frustum culling
* Single-threaded visibility determination
* Basic multithreaded visibility determination with per-frame worker thread creation
* Persistent-worker multithreaded visibility determination
* Configurable worker thread counts
* Shared visibility-test logic across culling implementations
* Dear ImGui runtime interface
* Runtime culling implementation selection
* Runtime scene configuration
* Real-time visibility statistics
* Real-time culling time, frame time and FPS measurements
* Culling correctness validation across implementations
* Configurable window resolution presets
* Keyboard shortcuts for runtime configuration

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
| `F1`        | Toggle camera mode                             |
| `F2`        | Cycle worker thread count                      |
| `F3`        | Cycle culling implementation                   |
| `F4`        | Cycle object-count preset                      |
| `F5`        | Run culling correctness validation             |
| `F6`        | Cycle resolution preset                        |
| `F10`       | Show / hide shortcut overlay                   |

### Runtime Interface
Dear ImGui provides runtime controls and performance information without requiring the application to be restarted.

The interface currently provides:
* Culling implementation selection
* Worker-thread count control
* Object-count preset selection
* Unlocked object-count control
* Total, visible and culled object counts
* Culling time
* Frame time
* FPS

### Freecam Mode
Freecam allows the camera to move freely through the rendered scene using the keyboard and mouse.
When Freecam is enabled, relative mouse mode is enabled so that the camera can rotate continuously without the cursor being constrained by the edges of the application window.

Note that Freecam has pitch limit of +-89 degrees to stop the camera from fliping over. Yaw is not unlimited.

### Static Mode
Static mode disables Freecam movement and mouse rotation.
Press `F1` to switch between Static and Freecam modes.
Static mode will be used for benchamrking.

### Visibility Determination
The framework currently provides three CPU-side frustum-culling implementations:
* **Single Thread** — sequential reference implementation.
* **Basic Multithreaded** — divides the scene between multiple worker threads created for each culling operation.
* **Persistent Multithreaded** — uses a persistent worker pool to avoid repeated thread creation and destruction between frames.

All implementations perform the same world-space AABB against camera-frustum visibility test, allowing their threading strategies to be compared under equivalent scene conditions.

### Scene Configuration
The framework supports configurable scene sizes for evaluating visibility-determination performance under different workloads.

Available object-count presets are:
* 100
* 500
* 1,000
* 2,500
* 5,000
* 7,500
* 10,000

Object count can also be unlocked and adjusted between 100 and 10,000 objects in increments of 100.
Scene generation is deterministic to provide repeatable object placement and colours between runs.

### Performance Monitoring
The framework includes a Dear ImGui performance overlay for monitoring the visibility determination system during development.
Press `F2` to cycle between culling configuration.

The overlay displays:
* Camera mode
* Total scene object count
* Visible object count
* Culled object count
* Active culling mode
* Active worker thread count
* Maximum available hardware threads
* Culling time
* Frame processing time
* Frames per second (FPS)

The culling configuration can be changed at runtime to compare the dedicated single-threaded implementation against the multithreaded implementation at increasing worker thread counts.

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
* Dear imgui

## Build
The project uses CMake. CMake automatically downloads project dependancies and manages them using FetchContent. These dependencies include:
* SDL3 3.2.16
* GLM 1.0.1
* GLAD 2.0.8
* Dear ImGUI 1.92.9b
* Tracy 0.11.1

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
