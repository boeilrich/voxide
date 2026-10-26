#include "entity.hpp"

auto Entity::getID() const noexcept -> std::uint64_t
{
    return entity_id;
}

auto Entity::getPosition() const noexcept -> glm::vec3
{
    return position_;
}

void Entity::setPosition(glm::vec3 position) noexcept
{
    position_ = position;
}

auto Entity::getYaw() const noexcept -> float
{
    return yaw_;
}

void Entity::setYaw(float yaw) noexcept
{
    yaw_ = yaw;
}

auto Entity::getPitch() const noexcept -> float
{
    return pitch_;
}

void Entity::setPitch(float pitch) noexcept
{
    pitch_ = pitch;
}
