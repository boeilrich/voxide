#include "camera.hpp"

#include <algorithm>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

Camera::Camera(glm::vec3 position, float yaw, float pitch, float fovy, float aspect_ratio) noexcept
    : position_(position),
      yaw_(yaw),
      pitch_(pitch),
      fovy_(fovy),
      aspect_ratio_(aspect_ratio)
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

    new_front_vector.x = std::cos(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
    new_front_vector.y = std::sin(glm::radians(pitch_));
    new_front_vector.z = std::sin(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));

    front_ = glm::normalize(new_front_vector);
    right_ = glm::normalize(glm::cross(front_, WORLD_UP));
    up_ = glm::normalize(glm::cross(right_, front_));
}

void Camera::handleLookInput(float xoffset, float yoffset, bool constrain_pitch = true) noexcept
{
    xoffset *= look_sensitivity_;
    yoffset *= look_sensitivity_;

    yaw_ += xoffset;
    pitch_ += yoffset;

    if (constrain_pitch) {
        pitch_ = std::min(pitch_, 89.999F);
        pitch_ = std::max(pitch_, -89.999F);
    }

    yaw_ = std::fmod(yaw_, 360.0F);

    updateDirectionVectors();
}

void Camera::setPosition(glm::vec3 position) noexcept
{
    position_ = position;
}

void Camera::setYaw(float yaw) noexcept
{
    yaw_ = yaw;
    updateDirectionVectors();
}

void Camera::setPitch(float pitch) noexcept
{
    pitch_ = pitch;
    updateDirectionVectors();
}

void Camera::setAspectRatio(float aspect_ratio) noexcept
{
    aspect_ratio_ = aspect_ratio;
}

void Camera::setLookSensitivity(float look_sensitivity) noexcept
{
    look_sensitivity_ = look_sensitivity;
}

auto Camera::getPosition() const noexcept -> glm::vec3
{
    return position_;
}

auto Camera::getYaw() const noexcept -> float
{
    return yaw_;
}

auto Camera::getPitch() const noexcept -> float
{
    return pitch_;
}

auto Camera::getFrontVector() const noexcept -> glm::vec3
{
    return front_;
}

auto Camera::getUpVector() const noexcept -> glm::vec3
{
    return up_;
}

auto Camera::getRightVector() const noexcept -> glm::vec3
{
    return right_;
}

auto Camera::getAspectRatio() const noexcept -> float
{
    return aspect_ratio_;
}

auto Camera::getLookSensitivity() const noexcept -> float
{
    return look_sensitivity_;
}

auto Camera::getViewMatrix() const noexcept -> glm::mat4
{
    return glm::lookAt(position_, position_ + front_, up_);
}

auto Camera::getProjectionMatrix() const noexcept -> glm::mat4
{
    return glm::perspective(glm::radians(fovy_), aspect_ratio_, NEAR_PLANE, FAR_PLANE);
}
