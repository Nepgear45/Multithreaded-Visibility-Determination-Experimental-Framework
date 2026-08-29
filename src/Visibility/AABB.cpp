#include "AABB.h"

#include <algorithm>
#include <limits>

AABB AABB::Transform(const glm::mat4& matrix) const
{
    const glm::vec3 corners[8] =
    {
        { min.x, min.y, min.z },
        { max.x, min.y, min.z },
        { min.x, max.y, min.z },
        { max.x, max.y, min.z },

        { min.x, min.y, max.z },
        { max.x, min.y, max.z },
        { min.x, max.y, max.z },
        { max.x, max.y, max.z }
    };

    glm::vec3 transformedMin(std::numeric_limits<float>::max());
    glm::vec3 transformedMax(std::numeric_limits<float>::lowest());

    for (const glm::vec3& corner : corners)
    {
        const glm::vec4 transformed = matrix * glm::vec4(corner, 1.0f);
        const glm::vec3 point(transformed);

        transformedMin = glm::min(transformedMin, point);
        transformedMax = glm::max(transformedMax, point);
    }

    return { transformedMin, transformedMax };
}