#pragma once

#include "SceneObject.h"

#include <vector>

class Scene
{
public:
    void GenerateGrid(int countX, int countY, int countZ, float spacing);
    const std::vector<SceneObject>& GetObjects() const;

private:
    std::vector<SceneObject> m_objects;
};