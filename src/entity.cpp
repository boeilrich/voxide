#include "entity.hpp"

auto Entity::getID() const noexcept -> std::uint64_t
{
    return m_entity_id;
}

auto Entity::getPosition() const noexcept -> glm::vec3
{
    return m_position;
}

void Entity::setPosition(glm::vec3 position) noexcept
{
    m_position = position;
}

auto Entity::getYaw() const noexcept -> float
{
    return m_yaw;
}

void Entity::setYaw(float yaw) noexcept
{
    m_yaw = yaw;
}

auto Entity::getPitch() const noexcept -> float
{
    return m_pitch;
}

void Entity::setPitch(float pitch) noexcept
{
    m_pitch = pitch;
}
