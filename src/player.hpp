#pragma once

#include <optional>

#include "camera.hpp"
#include "entity.hpp"

class Player : Entity
{
  private:
    std::optional<Camera> camera_;
    bool is_local_ = false;

  public:
    explicit Player(bool local = false);

    [[nodiscard]] auto isLocal() const noexcept -> bool;
    [[nodiscard]] auto getCamera() noexcept -> Camera*;

    void tick(float delta_time) override;
};
