#pragma once

#include <optional>

#include "camera.hpp"
#include "entity.hpp"

class Player : Entity
{
  private:
    std::optional<Camera> m_camera;
    bool m_local = false;

  public:
    explicit Player(bool local = false);

    [[nodiscard]] auto isLocal() const noexcept -> bool;
    [[nodiscard]] auto getCamera() noexcept -> Camera*;

    void tick(float delta_time) override;
};
