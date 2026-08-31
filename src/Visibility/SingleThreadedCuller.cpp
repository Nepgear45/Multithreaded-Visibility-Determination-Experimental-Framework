#include "SingleThreadedCuller.h"
#include "VisibilityTest.h"

#include <glm/gtc/matrix_transform.hpp>

// Test every scene object against the supplied camera frustum
void SingleThreadedCuller::Cull(const std::vector<SceneObject>& objects, const Frustum& frustum, std::vector<const SceneObject*>& visibleObjects) const
{
    // Remove visibility results from the previous frame
    visibleObjects.clear();

    // Reserve enough space for the worst case where every object is visible
    // This helps avoid repeated vector memory allocations while culling
    visibleObjects.reserve(objects.size());

    // Test each scene object one at a time on the calling thread
    for (const SceneObject& object : objects)
    {
        if (IsObjectVisible(object, frustum))
        {
            visibleObjects.push_back(&object);
        }
    }
}