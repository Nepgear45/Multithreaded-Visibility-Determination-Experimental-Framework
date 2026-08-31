#include "PersistentMultithreadedCuller.h"
#include "VisibilityTest.h"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

#include "AABB.h"

PersistentMultithreadedCuller::~PersistentMultithreadedCuller()
{
    Shutdown();
}

void PersistentMultithreadedCuller::Initialize(std::size_t maxThreadCount)
{
    // Prevent the worker pool from being initialized more than once
    if (!m_workers.empty()) return;

    // Always create at least one worker thread
    maxThreadCount = std::max<std::size_t>(1, maxThreadCount);

    // Create one private visibility list for every worker
    m_threadVisibleObjects.resize(maxThreadCount);

    // Reserve space so adding threads does not repeatedly reallocate
    m_workers.reserve(maxThreadCount);
    m_stop = false;

    // Create all worker threads once
    for (std::size_t workerIndex = 0; workerIndex < maxThreadCount; ++workerIndex)
    {
        m_workers.emplace_back(&PersistentMultithreadedCuller::WorkerLoop, this, workerIndex);
    }
}

void PersistentMultithreadedCuller::Shutdown()
{
    // Nothing to shut down if the worker pool was never created
    if (m_workers.empty()) return;

    { // Scope
        std::lock_guard<std::mutex> lock(m_mutex);

        // Tell every worker thread to exit
        m_stop = true;
    }

    // Wake every worker so they can see the shutdown request
    m_jobAvailable.notify_all();

    // Wait for every worker thread to finish
    for (std::thread& worker : m_workers)
    {
        if (worker.joinable()) worker.join();
    }

    m_workers.clear();
    m_threadVisibleObjects.clear();
}

void PersistentMultithreadedCuller::WorkerLoop(std::size_t workerIndex)
{
    // Remember the last culling job this worker processed
    std::size_t observedGeneration = 0;

    while (true)
    {
        const std::vector<SceneObject>* objects = nullptr;
        const Frustum* frustum = nullptr;

        std::size_t activeThreadCount = 0;

        {
            std::unique_lock<std::mutex> lock(m_mutex);

            // Sleep until either:
            //      - a new culling job becomes available
            //      - the application is shutting down
            // Also an interesting implementation
            m_jobAvailable.wait
            (
                lock,
                [this, observedGeneration]()
                {
                    return
                        m_stop ||
                        m_jobGeneration > observedGeneration;
                }
            );

            // Exit the worker thread during shutdown
            if (m_stop) return;

            // Remember which job this worker is processing
            observedGeneration = m_jobGeneration;

            activeThreadCount = m_activeThreadCount;

            // Workers above the requested thread count remain idle
            if (workerIndex >= activeThreadCount) continue;

            // Copy the current job information
            objects = m_objects;
            frustum = m_frustum;
        }

        const std::size_t objectCount = objects->size();

        // Divide the scene into approximately equal contiguous ranges
        const std::size_t objectsPerThread = (objectCount + activeThreadCount - 1) / activeThreadCount;
        const std::size_t startIndex = workerIndex * objectsPerThread;
        const std::size_t endIndex = std::min(startIndex + objectsPerThread,objectCount);

        // Get this worker's private result list
        std::vector<const SceneObject*>& localVisibleObjects = m_threadVisibleObjects[workerIndex];

        localVisibleObjects.clear();

        if (endIndex > startIndex) localVisibleObjects.reserve(endIndex - startIndex);

        // Test this worker's section of the scene
        for (std::size_t i = startIndex; i < endIndex; ++i)
        {
            const SceneObject& object = (*objects)[i];

            if (IsObjectVisible(object, *frustum))
            {
                localVisibleObjects.push_back(&object);
            }
        }

        { // Also scope
            std::lock_guard<std::mutex> lock(m_mutex);

            // Tell the main thread that this worker has finished
            ++m_completedWorkers;

            // Wake the main thread once every active worker is finished
            if (m_completedWorkers == m_activeThreadCount) m_jobFinished.notify_one();
        }
    }
}

void PersistentMultithreadedCuller::Cull(const std::vector<SceneObject>& objects, const Frustum& frustum, std::vector<const SceneObject*>& visibleObjects, std::size_t threadCount)
{
    visibleObjects.clear();

    // Nothing to process
    if (objects.empty()) return;

    // The persistent worker pool must be initialized first
    if (m_workers.empty()) return;

    // Always use at least one worker
    threadCount = std::max<std::size_t>(1, threadCount);

    // Never use more workers than were created
    threadCount = std::min(threadCount, m_workers.size());

    // Never use more workers than there are objects
    threadCount = std::min(threadCount, objects.size());

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Store the information required by the workers
        m_objects = &objects;
        m_frustum = &frustum;

        m_activeThreadCount = threadCount;
        m_completedWorkers = 0;

        // Incrementing this tells workers that a new job exists
        ++m_jobGeneration;
    }

    // Wake the worker threads
    m_jobAvailable.notify_all();

    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // Wait until every active worker has completed its range
        m_jobFinished.wait(
            lock,
            [this]()
            {
                return
                    m_completedWorkers ==
                    m_activeThreadCount;
            }
        );
    }

    // Reserve enough space for the final merged visibility list
    visibleObjects.reserve(objects.size());

    // Merge each worker's private results
    for (std::size_t workerIndex = 0; workerIndex < threadCount; ++workerIndex)
    {
        const std::vector<const SceneObject*>& localVisibleObjects = m_threadVisibleObjects[workerIndex];
        visibleObjects.insert( visibleObjects.end(), localVisibleObjects.begin(), localVisibleObjects.end());
    }
}