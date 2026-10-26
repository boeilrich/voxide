#include "player.hpp"

#include <SDL3/SDL_keyboard.h>

#include "camera.hpp"

Player::Player(bool local) : is_local_(local)
{
    if (local) {
        camera_.emplace(position_, yaw_, pitch_, 77.0F);
    }
}

auto Player::isLocal() const noexcept -> bool
{
    return is_local_;
}

auto Player::getCamera() noexcept -> Camera*
{
    if (!is_local_)
        return nullptr;
    return &camera_.value();
}

void Player::tick(float delta_time)
{
    if (is_local_) {
        camera_->setPosition(position_ + glm::vec3{0.0F, 1.8F, 0.0F});
        const bool* keys = SDL_GetKeyboardState(nullptr);

        const float sensitivity = 20.0F;

        if (keys[SDL_SCANCODE_LEFT]) {
            camera_->handleLookInput(-1000.0F * delta_time, 0.0F * delta_time, true);
        }
        if (keys[SDL_SCANCODE_RIGHT]) {
            camera_->handleLookInput(1000.0F * delta_time, 0.0F * delta_time, true);
        }

        if (keys[SDL_SCANCODE_UP]) {
            camera_->handleLookInput(0.0F * delta_time, 1000.0F * delta_time, true);
        }
        if (keys[SDL_SCANCODE_DOWN]) {
            camera_->handleLookInput(0.0F * delta_time, -1000.0F * delta_time, true);
        }

        if (keys[SDL_SCANCODE_W]) {
            setPosition(this->getPosition() + this->camera_->getFrontVector() * sensitivity * delta_time);
        }

        if (keys[SDL_SCANCODE_S]) {
            setPosition(this->getPosition() - this->camera_->getFrontVector() * sensitivity * delta_time);
        }

        if (keys[SDL_SCANCODE_D]) {
            setPosition(this->getPosition() + this->camera_->getRightVector() * sensitivity * delta_time);
        }

        if (keys[SDL_SCANCODE_A]) {
            setPosition(this->getPosition() - this->camera_->getRightVector() * sensitivity * delta_time);
        }

        if (keys[SDL_SCANCODE_SPACE]) {
            setPosition(this->getPosition() + glm::vec3(0.0, 1.0, 0.0) * sensitivity * delta_time);
        }

        if (keys[SDL_SCANCODE_LSHIFT]) {
            setPosition(this->getPosition() - glm::vec3(0.0, 1.0, 0.0) * sensitivity * delta_time);
        }
    }
}
