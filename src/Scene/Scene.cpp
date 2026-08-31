#include "Scene.h"
#include <random>
#include <cmath>

void Scene::GenerateGrid(std::size_t objectCount, float spacing)
{
    m_objects.clear();
    m_objects.reserve(objectCount);

    // Fixed seed for deterministic colours
    std::mt19937 randomGenerator(42);
    std::uniform_real_distribution<float> colourDistribution(0.2f, 1.0f);

    // Calculate grid size needed to fit all objects
    const std::size_t gridSize = static_cast<std::size_t>( std::ceil(std::cbrt(static_cast<double>(objectCount))));

    // Centre the grid around the origin
    const float gridOffset = static_cast<float>(gridSize - 1) * spacing * 0.5f;

    for (std::size_t x = 0; x < gridSize; ++x)
    {
        for (std::size_t y = 0; y < gridSize; ++y)
        {
            for (std::size_t z = 0; z < gridSize; ++z)
            {
                if (m_objects.size() >= objectCount) return;

                SceneObject object;

                object.position =
                {
                    static_cast<float>(x) * spacing - gridOffset,
                    static_cast<float>(y) * spacing - gridOffset,
                    static_cast<float>(z) * spacing - gridOffset
                };

                object.colour =
                {
                    colourDistribution(randomGenerator),
                    colourDistribution(randomGenerator),
                    colourDistribution(randomGenerator)
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