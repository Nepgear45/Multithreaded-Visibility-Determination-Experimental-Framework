#pragma once

#include <array>

#include <glm/glm.hpp>

#include "AABB.h"

struct Plane
{
    glm::vec3 normal{ 0.0f };
    float distance = 0.0f;

    void Normalize();
};

class Frustum
{
public:
    enum PlaneIndex
    {
        Left = 0,
        Right,
        Bottom,
        Top,
        Near,
        Far
    };

    static Frustum FromViewProjection(const glm::mat4& viewProjection);

    bool Intersects(const AABB& worldBounds) const;

private:
    std::array<Plane, 6> m_planes;
};