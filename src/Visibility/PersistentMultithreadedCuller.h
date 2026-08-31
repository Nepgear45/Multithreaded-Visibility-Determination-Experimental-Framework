#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <thread>
#include <vector>

#include "Frustum.h"
#include "Scene/SceneObject.h"

class PersistentMultithreadedCuller
{
public:
    PersistentMultithreadedCuller() = default;

    ~PersistentMultithreadedCuller();

    // Prevent accidental copying because this class owns worker threads
    PersistentMultithreadedCuller(const PersistentMultithreadedCuller&) = delete;

    PersistentMultithreadedCuller& operator=(const PersistentMultithreadedCuller&) = delete;

    // Create the persistent worker threads
    void Initialize(std::size_t maxThreadCount);

    // Stop and destroy all worker threads
    void Shutdown();

    // Perform one visibility determination job
    void Cull(const std::vector<SceneObject>& objects,const Frustum& frustum,std::vector<const SceneObject*>& visibleObjects,std::size_t threadCount);

private:
    // Function executed continuously by each worker thread
    void WorkerLoop(std::size_t workerIndex);

    // Persistent worker threads
    std::vector<std::thread> m_workers;

    // Each worker writes to its own visibility list
    std::vector<std::vector<const SceneObject*>> m_threadVisibleObjects;

    // Synchronisation used to distribute work
    std::mutex m_mutex;

    std::condition_variable m_jobAvailable;
    std::condition_variable m_jobFinished;

    // Signals that the worker threads should exit
    bool m_stop = false;

    // Incremented each time a new culling job is submitted
    std::size_t m_jobGeneration = 0;

    // Number of workers that have completed the current job
    std::size_t m_completedWorkers = 0;

    // Number of workers being used for the current job
    std::size_t m_activeThreadCount = 0;

    // Current scene data being processed
    const std::vector<SceneObject>* m_objects = nullptr;

    // Current camera frustum being processed
    const Frustum* m_frustum = nullptr;
};