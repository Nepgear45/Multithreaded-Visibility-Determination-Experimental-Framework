#include "SingleThreadedCuller.h"

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
        // Build this object's model matrix using its position, rotation and scale
        // The same transform is used to calculate the object's world-space AABB
        glm::mat4 model{ 1.0f };

        model = glm::translate(model, object.position);
        model = glm::rotate(model, glm::radians(object.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(object.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(object.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, object.scale);

        // Transform the object's local-space AABB into world-space
        const AABB worldBounds = object.localBounds.Transform(model);

        // Skip objects that are completely outside the camera frustum
        if (!frustum.Intersects(worldBounds)) continue;

        // Store a pointer to the object if it passed the frustum test
        visibleObjects.push_back(&object);
    }
}