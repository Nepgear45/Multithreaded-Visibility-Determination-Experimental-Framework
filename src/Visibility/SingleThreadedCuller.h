#pragma once

#include <vector>

#include "Frustum.h"
#include "Scene/SceneObject.h"

class SingleThreadedCuller
{
public:
    void Cull(const std::vector<SceneObject>& objects, const Frustum& frustum, std::vector<const SceneObject*>& visibleObjects) const;
};