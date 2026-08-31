#pragma once

#include "SceneObject.h"

#include <vector>

class Scene
{
public:
    void GenerateGrid(std::size_t objectCount, float spacing);
    const std::vector<SceneObject>& GetObjects() const;

private:
    std::vector<SceneObject> m_objects;
};