#include "Frustum.h"

#include <glm/geometric.hpp>

void Plane::Normalize()
{
    const float length = glm::length(normal);

    if (length > 0.0f)
    {
        normal /= length;
        distance /= length;
    }
}

Frustum Frustum::FromViewProjection(const glm::mat4& matrix)
{
    Frustum frustum;

    // GLM uses column-major matrices.
    // Extracting rows explicitly.
    const glm::vec4 row0
    (
        matrix[0][0],
        matrix[1][0],
        matrix[2][0],
        matrix[3][0]
    );

    const glm::vec4 row1
    (
        matrix[0][1],
        matrix[1][1],
        matrix[2][1],
        matrix[3][1]
    );

    const glm::vec4 row2
    (
        matrix[0][2],
        matrix[1][2],
        matrix[2][2],
        matrix[3][2]
    );

    const glm::vec4 row3
    (
        matrix[0][3],
        matrix[1][3],
        matrix[2][3],
        matrix[3][3]
    );

    const glm::vec4 planes[6] =
    {
        row3 + row0, // Left
        row3 - row0, // Right
        row3 + row1, // Bottom
        row3 - row1, // Top
        row3 + row2, // Near
        row3 - row2  // Far
    };

    for (int i = 0; i < 6; ++i)
    {
        frustum.m_planes[i].normal = glm::vec3(planes[i]);
        frustum.m_planes[i].distance = planes[i].w;
        frustum.m_planes[i].Normalize();
    }

    return frustum;
}

bool Frustum::Intersects(const AABB& bounds) const
{
    for (const Plane& plane : m_planes)
    {
        glm::vec3 positiveVertex;

        positiveVertex.x = plane.normal.x >= 0.0f ? bounds.max.x : bounds.min.x;
        positiveVertex.y = plane.normal.y >= 0.0f ? bounds.max.y : bounds.min.y;
        positiveVertex.z = plane.normal.z >= 0.0f ? bounds.max.z : bounds.min.z;

        const float distance = glm::dot(plane.normal, positiveVertex) + plane.distance;

        if (distance < 0.0f) return false;
    }

    return true;
}