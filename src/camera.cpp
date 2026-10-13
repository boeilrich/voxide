#include "camera.hpp"

#include <algorithm>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

Camera::Camera(glm::vec3 position, float yaw, float pitch, float fovy, float aspect_ratio) noexcept
    : m_position(position),
      m_yaw(yaw),
      m_pitch(pitch),
      m_fovy(fovy),
      m_aspect_ratio(aspect_ratio)
{
    spdlog::trace("Creating Camera: \n > x={}, y={}, z={}\n > yaw={}, pitch={}\n > fovy={}\n > aspect-ratio={}",
                  position.x, position.y, position.z, yaw, pitch, fovy, aspect_ratio);

    updateDirectionVectors();
}

Camera::~Camera()
{
    spdlog::trace("Destroying Camera");
}

void Camera::updateDirectionVectors() noexcept
{
    glm::vec3 new_front_vector;

    new_front_vector.x = std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    new_front_vector.y = std::sin(glm::radians(m_pitch));
    new_front_vector.z = std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));

    m_front = glm::normalize(new_front_vector);
    m_right = glm::normalize(glm::cross(m_front, WORLD_UP));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::handleLookInput(float xoffset, float yoffset, bool constrain_pitch = true) noexcept
{
    xoffset *= m_look_sensitivity;
    yoffset *= m_look_sensitivity;

    m_yaw += xoffset;
    m_pitch += yoffset;

    if (constrain_pitch) {
        m_pitch = std::min(m_pitch, 89.999F);
        m_pitch = std::max(m_pitch, -89.999F);
    }

    m_yaw = std::fmod(m_yaw, 360.0F);

    updateDirectionVectors();
}

void Camera::setPosition(glm::vec3 position) noexcept
{
    m_position = position;
}

void Camera::setYaw(float yaw) noexcept
{
    m_yaw = yaw;
    updateDirectionVectors();
}

void Camera::setPitch(float pitch) noexcept
{
    m_pitch = pitch;
    updateDirectionVectors();
}

void Camera::setAspectRatio(float aspect_ratio) noexcept
{
    m_aspect_ratio = aspect_ratio;
}

void Camera::setLookSensitivity(float look_sensitivity) noexcept
{
    m_look_sensitivity = look_sensitivity;
}

auto Camera::getPosition() const noexcept -> glm::vec3
{
    return m_position;
}

auto Camera::getYaw() const noexcept -> float
{
    return m_yaw;
}

auto Camera::getPitch() const noexcept -> float
{
    return m_pitch;
}

auto Camera::getFrontVector() const noexcept -> glm::vec3
{
    return m_front;
}

auto Camera::getUpVector() const noexcept -> glm::vec3
{
    return m_up;
}

auto Camera::getRightVector() const noexcept -> glm::vec3
{
    return m_right;
}

auto Camera::getAspectRatio() const noexcept -> float
{
    return m_aspect_ratio;
}

auto Camera::getLookSensitivity() const noexcept -> float
{
    return m_look_sensitivity;
}

auto Camera::getViewMatrix() const noexcept -> glm::mat4
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

auto Camera::getProjectionMatrix() const noexcept -> glm::mat4
{
    return glm::perspective(glm::radians(m_fovy), m_aspect_ratio, NEAR_PLANE, FAR_PLANE);
}
