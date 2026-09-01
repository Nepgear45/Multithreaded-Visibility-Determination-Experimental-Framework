#pragma once

#include <SDL3/SDL.h>
#include <cstddef>
#include <vector>

#include "Rendering/Shader.h"
#include "Camera/Camera.h"
#include "Scene/Scene.h"
#include "Visibility/SingleThreadedCuller.h"
#include "Visibility/MultithreadedCuller.h"
#include "Visibility/PersistentMultithreadedCuller.h"

enum class CameraMode {Static, Freecam};
enum class CullingMode {SingleThreaded, Multithreaded, PersistentMultithreaded};
enum class BenchmarkState {Inactive, Running};
enum class BenchmarkPhase {Validation, Warmup, Measurement};

struct BenchmarkConfiguration
{
    CullingMode cullingMode;
    std::size_t objectCount;
    std::size_t threadCount;
};

class Application
{
public:
    Application();
    ~Application();

    bool Initialise();
    void Run();
    void Shutdown();

    // Camera Variables
    Camera m_camera;

    CameraMode m_cameraMode = CameraMode::Freecam;

    float m_deltaTime = 0.0f;
    Uint64 m_lastFrameTime = 0;

    Scene m_scene;

    double m_cullingTimeMs = 0.0;
    double m_frameTimeMs = 0.0;
    double m_fps = 0.0;

    // Current number of worker threads used by the multithreaded culler
    std::size_t m_threadCount = 1;

    // Maximum number of concurrent threads reported by the CPU
    std::size_t m_maxThreadCount = 1;

    CullingMode m_cullingMode = CullingMode::SingleThreaded;

    // Benchmark variables
    BenchmarkState m_benchmarkState = BenchmarkState::Inactive;

    bool m_benchmarkRunning = false;
    bool m_benchmarkAbortRequested = false;

    std::vector<BenchmarkConfiguration> m_benchmarkConfigurations;
    BenchmarkPhase m_benchmarkPhase = BenchmarkPhase::Validation;

    std::size_t m_totalBenchmarkTests = 0;
    std::size_t m_currentBenchmarkTest = 0;
    std::size_t m_currentBenchmarkSample = 0;
    std::size_t m_samplesPerBenchmarkTest = 1000;

    std::size_t m_currentWarmupFrame = 0;
    std::size_t m_warmupFramesPerBenchmarkTest = 300;

    void StartBenchmark();
    void StopBenchmark();
    void BuildBenchmarkConfigurations();
    void ApplyBenchmarkConfiguration();
    void UpdateBenchmark(const Frustum& frustum);

    void RenderBenchmarkUI();
    void PrintBenchmarkConfiguration() const;
    void PrintBenchmarkResults() const;

private:
    double m_benchmarkCullingTimeTotal = 0.0;
    double m_benchmarkFrameTimeTotal = 0.0;

    void ProcessEvents();
    void Update();
    void Render();
    void RegenerateScene();

    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;

    GLuint m_triangleVAO = 0;
    GLuint m_triangleVBO = 0;

    Shader m_triangleShader;

    SingleThreadedCuller m_singleThreadedCuller;
    MultithreadedCuller m_multithreadedCuller;
    PersistentMultithreadedCuller m_persistentMultithreadedCuller;

    // All visibile objects
    std::vector<const SceneObject*> m_visibleObjects;

    // Determines whether arbitrary object counts can be selected
    bool m_unlockedObjectCount = false;

    // Object count selected in the UI
    std::size_t m_requestedObjectCount = 1000;

    // Currently selected preset
    std::size_t m_objectCountPresetIndex = 2;

    bool m_running = false;
    bool m_showShortcuts = false;

    int m_windowWidth = 1920;
    int m_windowHeight = 1080;
    int m_resolutionIndex = 2;

    void SetWindowResolution(int width, int height);
    void CycleResolution();

    void UpdateWindowTitle(std::size_t visibleObjects,std::size_t totalObjects);
    void RenderPerformanceOverlay(std::size_t visibleObjects,std::size_t totalObjects);

    void CycleThreadCount();
    void CycleCullingMode();
    void CycleObjectCountPreset();
    void RenderShortcutsOverlay();

    bool m_validationRequested = false;
    bool m_hasValidationResult = false;
    bool m_cullingResultsMatch = false;

    std::size_t m_validationSingleVisible = 0;
    std::size_t m_validationBasicVisible = 0;
    std::size_t m_validationPersistentVisible = 0;
    std::size_t m_validationObjectCount = 0;

    bool ValidateCullingResults(const Frustum& frustum);
};