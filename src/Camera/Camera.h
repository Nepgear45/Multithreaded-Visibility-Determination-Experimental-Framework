#pragma once

#include <glm/glm.hpp>

enum class CameraMovement
{
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};

class Camera
{
public:
    Camera
    (
        const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f),
        const glm::vec3& worldUp = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f,
        float pitch = 0.0f
    );

    glm::mat4 GetViewMatrix() const;

    void ProcessKeyboard(CameraMovement direction, float deltaTime);
    void ProcessMouseMovement(float xOffset, float yOffset);

    void SetMovementSpeed(float speed);
    void SetMouseSensitivity(float sensitivity);

    void SetTransform(const glm::vec3& position, float yaw, float pitch);

    const glm::vec3& GetPosition() const;

private:
    void UpdateCameraVectors();

    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;

    float m_yaw;
    float m_pitch;

    float m_movementSpeed = 5.0f;
    float m_mouseSensitivity = 0.1f;
};