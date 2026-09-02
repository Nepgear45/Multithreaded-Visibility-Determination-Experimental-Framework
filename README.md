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
* Automated benchmark configuration sequencing
* Benchmark validation, warm-up and measurement phases
* Per-test culling correctness validation
* Deterministic benchmark camera configuration
* Fixed benchmark resolution
* Automated benchmark test progression
* Benchmark progress and configuration interface
* Per-test average performance summary
* Benchmark abort controls
* Per-frame benchmark sample collection
* Raw benchmark sample export to timestamped CSV files
* Benchmark run identification using timestamps
* Visibility-count stability checking during measurement
* Benchmark warm-up and configurable measurement sample counts

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
| `F7`        | Start benchmarking                             |
| `F8`        | Abort benchmarking                             |
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
* Culling correctness validation status
* Benchmark progress and current configuration
* Benchmark abort control

### Freecam Mode
Freecam allows the camera to move freely through the rendered scene using the keyboard and mouse.
When Freecam is enabled, relative mouse mode is enabled so that the camera can rotate continuously without the cursor being constrained by the edges of the application window.

Note that Freecam limits pitch to ±89 degrees to prevent the camera from flipping over. Yaw is unrestricted.

### Static Mode
Static mode disables Freecam movement and mouse rotation.
Press `F1` to switch between Static and Freecam modes.

Automated benchmarks use a fixed deterministic camera configuration independently of the interactive camera mode.

### Visibility Determination
The framework currently provides three CPU-side frustum-culling implementations:
* **Single Thread** — sequential reference implementation.
* **Basic Multithreaded** — divides the scene between multiple worker threads created for each culling operation.
* **Persistent Multithreaded** — uses a persistent worker pool to avoid repeated thread creation and destruction between frames.

All implementations use the same world-space AABB against camera-frustum visibility test. This keeps the visibility tests consistent so that the primary experimental variable is the threading strategy.

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

The runtime controls can be used during development to compare the single-threaded, basic multithreaded and persistent-worker implementations across different worker-thread and scene configurations.

## Benchmarking

The framework includes an automated benchmark system for collecting repeatable performance measurements across different visibility-determination configurations.

Each benchmark configuration progresses through three phases:

1. **Validation** — verifies that all culling implementations return the same visible objects.
2. **Warm-up** — allows the current configuration to execute before measurements are recorded.
3. **Measurement** — records individual performance samples for later analysis.

Benchmark execution uses a fixed camera configuration and fixed display resolution to keep these conditions consistent between tests. VSync is disabled so that frame presentation does not impose a refresh-rate limit on performance measurements.

For each measured frame, the framework records:

* Test index
* Sample index
* Culling implementation
* Object count
* Worker-thread count
* Visible-object count
* Culling time in milliseconds
* CPU-side frame processing time in milliseconds

Raw samples are written to timestamped CSV files under:

```text
results/raw/
```

```markdown
## Technologies Used
* x86-64 architecture
* C++20
* Python 3.13
* CMake
* SDL3
* OpenGL 4.6 Core
* GLAD 2.0.8
* GLM
* Tracy Profiler
* Dear ImGui
* Visual Studio 2022
```

## Build

## Prerequisites
The following software is required to configure and build the project on Windows.

### 1. Visual Studio 2022
Install **Visual Studio 2022 Community** or another Visual Studio 2022 edition.

During installation, enable the following workload:

```text
Desktop development with C++
```

Make sure the installation includes:

* MSVC C++ build tools
* Windows 10 or Windows 11 SDK
* C++ CMake tools for Windows

Visual Studio provides the C and C++ compiler toolchain required by CMake.

### 2. CMake
Install CMake and make sure it is available from the command line.

Verify the installation with:

```powershell
cmake --version
```

### 3. Git
Git is required because CMake FetchContent downloads project dependencies from Git repositories.

Verify the installation with:

```powershell
git --version
```

If Git has just been installed, close and reopen PowerShell or Visual Studio so the updated PATH is detected.

### 4. Python 3.13
Python 3.13 is required during configuration because GLAD 2.0.8 generates the OpenGL loader source code using Python.

Verify the installation with:

```powershell
python --version
```

The project has been developed using Python 3.13.

## Prerequisites

The following software is required to configure and build the project on Windows.

### Visual Studio 2022

Install **Visual Studio 2022 Community** or another Visual Studio 2022 edition.

During installation, enable the following workload:

```text
Desktop development with C++
```

Make sure the installation includes:

* MSVC C++ build tools
* Windows 10 or Windows 11 SDK
* C++ CMake tools for Windows

### CMake

Install CMake and make sure it is available from PowerShell.

Verify the installation with:

```powershell
cmake --version
```

### Git

Git is required because CMake FetchContent downloads project dependencies from Git repositories.

Verify the installation with:

```powershell
git --version
```

### Python 3.13

Python 3.13 is required during configuration for GLAD generation.

Verify the installation with:

```powershell
python --version
```

### Jinja2

Jinja2 is required by GLAD during OpenGL loader generation.

Install Jinja2 using the same Python 3.13 installation used by CMake:

```powershell
& "$env:LOCALAPPDATA\Programs\Python\Python313\python.exe" -m pip install Jinja2
```

Verify the installation with:

```powershell
& "$env:LOCALAPPDATA\Programs\Python\Python313\python.exe" -m pip show Jinja2
```

## Configure and Build

Open PowerShell and navigate to the project root directory containing `CMakeLists.txt`.

For example:

```powershell
cd "C:\Path\To\MultithreadedVisibilityFramework"
```

### Configure

Configure the project with:

```powershell
cmake -S . -B build -DPython_EXECUTABLE="$env:LOCALAPPDATA\Programs\Python\Python313\python.exe"
```

CMake will configure the project and download the required dependencies using FetchContent.

### Build Release

After configuration completes successfully, build the Release configuration with:

```powershell
cmake --build build --config Release
```

The Release executable will normally be generated under:

```text
build\Release\
```

To locate the executable:

```powershell
Get-ChildItem -Path build -Recurse -Filter *.exe | Select-Object FullName
```

Formal benchmark runs should use the Release executable and should be launched without the Visual Studio debugger attached.

### Clean Build

If a configuration fails or the build directory needs to be regenerated, delete it with:

```powershell
Remove-Item -Recurse -Force build
```

Then run the configure and Release build commands again:

```powershell
cmake -S . -B build -DPython_EXECUTABLE="$env:LOCALAPPDATA\Programs\Python\Python313\python.exe"
cmake --build build --config Release
```

### Verify Required Tools

Before configuring the project on a new machine, the required tools can be checked with:

```powershell
cmake --version
git --version
python --version
python -m pip show Jinja2
```
