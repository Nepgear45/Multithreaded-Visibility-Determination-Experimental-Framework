#pragma once

#include <cstddef>
#include <vector>

#include "Frustum.h"
#include "Scene/SceneObject.h"

class MultithreadedCuller
{
public:
    void Cull(const std::vector<SceneObject>& objects, const Frustum& frustum, std::vector<const SceneObject*>& visibleObjects, std::size_t threadCount) const;
};