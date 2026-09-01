#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>


Camera::Camera
(
    const glm::vec3& position,
    const glm::vec3& worldUp,
    float yaw,
    float pitch
) :
    m_position(position),
    m_worldUp(worldUp),
    m_yaw(yaw),
    m_pitch(pitch)
{
    UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
    const float velocity = m_movementSpeed * deltaTime;

    // Directions to move
    switch (direction)
    {
    case CameraMovement::Forward:
        m_position += m_front * velocity;
        break;

    case CameraMovement::Backward:
        m_position -= m_front * velocity;
        break;

    case CameraMovement::Left:
        m_position -= m_right * velocity;
        break;

    case CameraMovement::Right:
        m_position += m_right * velocity;
        break;

    case CameraMovement::Up:
        m_position += m_worldUp * velocity;
        break;

    case CameraMovement::Down:
        m_position -= m_worldUp * velocity;
        break;
    }
}

void Camera::ProcessMouseMovement(
    float xOffset,
    float yOffset
)
{
    xOffset *= m_mouseSensitivity;
    yOffset *= m_mouseSensitivity;

    m_yaw += xOffset;
    m_pitch += yOffset;
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

    UpdateCameraVectors();
}

void Camera::SetMovementSpeed(float speed)
{
    m_movementSpeed = speed;
}

void Camera::SetMouseSensitivity(float sensitivity)
{
    m_mouseSensitivity = sensitivity;
}

const glm::vec3& Camera::GetPosition() const
{
    return m_position;
}

void Camera::UpdateCameraVectors()
{
    glm::vec3 front;

    front.x = std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    front.y = std::sin(glm::radians(m_pitch));
    front.z = std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));

    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::SetTransform(const glm::vec3& position, float yaw, float pitch)
{
    m_position = position;
    m_yaw = yaw;
    m_pitch = std::clamp(pitch, -89.0f, 89.0f);

    UpdateCameraVectors();
}