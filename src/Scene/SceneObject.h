#pragma once

#include <glm/glm.hpp>

#include "Visibility/AABB.h"

struct SceneObject
{
    glm::vec3 position{ 0.0f };
    glm::vec3 rotation{ 0.0f };
    glm::vec3 scale{ 1.0f };

    AABB localBounds
    {
        glm::vec3(-0.5f),
        glm::vec3(0.5f)
    };
};