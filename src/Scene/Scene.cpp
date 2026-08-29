#include "Scene.h"
#include <random>

void Scene::GenerateGrid(int countX, int countY, int countZ, float spacing)
{
    // Used a fixed seed so the same colours are generated every time
    std::mt19937 randomGenerator(42);

    // Avoid very dark colours so objects remain easy to see (0.2 ~ 1.0)
    std::uniform_real_distribution<float> colourDistribution(0.2f, 1.0f);

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

                // Gives each object a deterministic random colour
                object.colour =
                {
                    colourDistribution(randomGenerator),
                    colourDistribution(randomGenerator),
                    colourDistribution(randomGenerator)
                };

                // TEMPORARY TEST:
                // Stretch every cube along the X axis
                object.scale =
                {
                    1.2f,
                    1.2f,
                    1.2f
                };

                // TEMPORARY TEST:
                // Rotate every cube 45 degrees around the Y axis
                object.rotation =
                {
                    0.0f,
                    45.0f,
                    0.0f
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