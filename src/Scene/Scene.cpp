#include "Scene.h"

void Scene::GenerateGrid(int countX, int countY, int countZ, float spacing)
{
    m_objects.clear();

    const int totalObjects = countX * countY * countZ;

    m_objects.reserve(totalObjects);

    const float offsetX = (countX - 1) * spacing * 0.5f;
    const float offsetY = (countY - 1) * spacing * 0.5f;
    const float offsetZ = (countZ - 1) * spacing * 0.5f;

    for (int z = 0; z < countZ; ++z)
    {
        for (int y = 0; y < countY; ++y)
        {
            for (int x = 0; x < countX; ++x)
            {
                SceneObject object;

                object.position =
                {
                    x * spacing - offsetX,
                    y * spacing - offsetY,
                    z * spacing - offsetZ
                };

                m_objects.push_back(object);
            }
        }
    }
}

const std::vector<SceneObject>&Scene::GetObjects() const
{
    return m_objects;
}