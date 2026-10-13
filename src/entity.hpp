#pragma once

#include <atomic>

#include <glm/glm.hpp>

class Entity
{
  protected:
    static inline std::atomic<std::uint64_t> m_id_counter{0};
    std::uint64_t m_entity_id = m_id_counter.fetch_add(1, std::memory_order_relaxed);

    glm::vec3 m_position = {0.0F, 0.0F, 0.0F};
    float m_yaw = 0.0, m_pitch = 0.0;

  public:
    Entity() = default;
    virtual ~Entity() = default;

    Entity(const Entity&) = default;
    Entity(Entity&&) = delete;
    auto operator=(const Entity&) -> Entity& = default;
    auto operator=(Entity&&) -> Entity& = delete;

    virtual void tick(float delta_time) = 0;

    [[nodiscard]] auto getID() const noexcept -> std::uint64_t;

    [[nodiscard]] auto getPosition() const noexcept -> glm::vec3;
    void setPosition(glm::vec3 position) noexcept;

    [[nodiscard]] auto getYaw() const noexcept -> float;
    void setYaw(float yaw) noexcept;

    [[nodiscard]] auto getPitch() const noexcept -> float;
    void setPitch(float pitch) noexcept;
};
