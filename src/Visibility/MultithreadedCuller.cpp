#include "MultithreadedCuller.h"
#include "VisibilityTest.h"

#include <algorithm>
#include <thread>

#include <glm/gtc/matrix_transform.hpp>

void MultithreadedCuller::Cull(const std::vector<SceneObject>& objects, const Frustum& frustum, std::vector<const SceneObject*>& visibleObjects, std::size_t threadCount) const
{
    // Remove visibility results from the previous frame
    visibleObjects.clear();

    // If there are no objects to test, there is nothing to do
    if (objects.empty()) return;

    // Make sure at least one worker thread is used
    threadCount = std::max<std::size_t>(1, threadCount);

    // Do not create more worker threads than there are scene objects
    threadCount = std::min(threadCount, objects.size());

    // Create one result list for each worker thread
    // Each thread writes only to its own vector to avoid synchronization
    std::vector<std::vector<const SceneObject*>> threadVisibleObjects(threadCount);

    // Store the worker threads so they can all be joined before continuing
    std::vector<std::thread> workers;
    workers.reserve(threadCount);

    // Calculate how many objects each worker thread should process
    // The final thread may process fewer objects if the division is uneven
    const std::size_t objectsPerThread = (objects.size() + threadCount - 1) / threadCount;

    // Create the worker threads
    for (std::size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        // Calculate the object range assigned to this worker thread
        const std::size_t startIndex = threadIndex * objectsPerThread;
        const std::size_t endIndex = std::min(startIndex + objectsPerThread, objects.size());

        // Stop creating workers if all objects have already been assigned
        if (startIndex >= objects.size()) break;

        // Run this object's range on a separate worker thread
        workers.emplace_back
        (
            [&, threadIndex, startIndex, endIndex]()
            {
                // Get this worker thread's private visibility result list
                std::vector<const SceneObject*>& localVisibleObjects = threadVisibleObjects[threadIndex];

                // Reserve enough space for the maximum number of objects this worker could mark as visible
                localVisibleObjects.reserve(endIndex - startIndex);

                // Test every scene object assigned to this worker thread
                for (std::size_t i = startIndex; i < endIndex; ++i)
                {
                    const SceneObject& object = objects[i];

                    if (IsObjectVisible(object, frustum))
                    {
                        localVisibleObjects.push_back(&object);
                    }
                }
            }
        );
    }

    // Wait for every worker thread to finish its assigned object range
    for (std::thread& worker : workers) worker.join();

    // Reserve enough space for the worst case where every object is visible
    visibleObjects.reserve(objects.size());

    // Combine the visibility results produced by each worker thread
    for (const std::vector<const SceneObject*>& localVisibleObjects : threadVisibleObjects) visibleObjects.insert(visibleObjects.end(), localVisibleObjects.begin(), localVisibleObjects.end());
}