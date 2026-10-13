#include "player.hpp"

#include <SDL3/SDL_keyboard.h>

#include "camera.hpp"

Player::Player(bool local) : m_local(local)
{
    if (local) {
        m_camera.emplace(m_position, m_yaw, m_pitch, 77.0F);
    }
}

auto Player::isLocal() const noexcept -> bool
{
    return m_local;
}

auto Player::getCamera() noexcept -> Camera*
{
    if (!m_local)
        return nullptr;
    return &m_camera.value();
}

void Player::tick(float delta_time)
{
    if (m_local) {
        m_camera->setPosition(m_position + glm::vec3{0.0F, 1.8F, 0.0F});
        const bool* keys = SDL_GetKeyboardState(nullptr);

        const float sensitivity = 20.0F;

        if (keys[SDL_SCANCODE_LEFT]) {
            m_camera->handleLookInput(-1000.0F * delta_time, 0.0F * delta_time, true);
        }
        if (keys[SDL_SCANCODE_RIGHT]) {
            m_camera->handleLookInput(1000.0F * delta_time, 0.0F * delta_time, true);
        }

        if (keys[SDL_SCANCODE_UP]) {
            m_camera->handleLookInput(0.0F * delta_time, 1000.0F * delta_time, true);
        }
        if (keys[SDL_SCANCODE_DOWN]) {
            m_camera->handleLookInput(0.0F * delta_time, -1000.0F * delta_time, true);
        }

        if (keys[SDL_SCANCODE_W]) {
            setPosition(this->getPosition() + this->m_camera->getFrontVector() * sensitivity * delta_time);
        }

        if (keys[SDL_SCANCODE_S]) {
            setPosition(this->getPosition() - this->m_camera->getFrontVector() * sensitivity * delta_time);
        }

        if (keys[SDL_SCANCODE_D]) {
            setPosition(this->getPosition() + this->m_camera->getRightVector() * sensitivity * delta_time);
        }

        if (keys[SDL_SCANCODE_A]) {
            setPosition(this->getPosition() - this->m_camera->getRightVector() * sensitivity * delta_time);
        }

        if (keys[SDL_SCANCODE_SPACE]) {
            setPosition(this->getPosition() + glm::vec3(0.0, 1.0, 0.0) * sensitivity * delta_time);
        }

        if (keys[SDL_SCANCODE_LSHIFT]) {
            setPosition(this->getPosition() - glm::vec3(0.0, 1.0, 0.0) * sensitivity * delta_time);
        }
    }
}
