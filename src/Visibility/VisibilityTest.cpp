#include "VisibilityTest.h"

#include <glm/gtc/matrix_transform.hpp>

bool IsObjectVisible(const SceneObject& object, const Frustum& frustum)
{
    // Build Model Matrix
    glm::mat4 model{ 1.0f };
    model = glm::translate(model, object.position);
    model = glm::rotate(model, glm::radians(object.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(object.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(object.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, object.scale);

    // Transform Bounds
    const AABB worldBounds = object.localBounds.Transform(model);

    // Test Against Frustum
    return frustum.Intersects(worldBounds);
}