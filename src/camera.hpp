#pragma once

#include <glm/glm.hpp>

constexpr glm::vec3 WORLD_UP = {0.0F, 1.0F, 0.0F};
constexpr glm::vec3 DEFAULT_FRONT = {0.0F, 0.0F, -1.0F};
constexpr glm::vec3 DEFAULT_RIGHT = {1.0F, 0.0F, 0.0F};

constexpr float DEFAULT_YAW = -90.0F;
constexpr float DEFAULT_PITCH = 0.0F;
constexpr float DEFAULT_FOVY = 90.0F;
constexpr float DEFAULT_ASPECT_RATIO = 16.0F / 9.0F;
constexpr float DEFAULT_LOOK_SENSITIVITY = 0.1F;

constexpr float NEAR_PLANE = 0.01F;
constexpr float FAR_PLANE = 1000.0F;

class Camera
{
  private:
    glm::vec3 m_position;
    float m_yaw = DEFAULT_YAW;
    float m_pitch = DEFAULT_PITCH;

    float m_fovy = DEFAULT_FOVY;
    float m_aspect_ratio = DEFAULT_ASPECT_RATIO;

    float m_look_sensitivity = DEFAULT_LOOK_SENSITIVITY;

    glm::vec3 m_up = WORLD_UP;
    glm::vec3 m_front = DEFAULT_FRONT;
    glm::vec3 m_right = DEFAULT_RIGHT;

    void updateDirectionVectors() noexcept;

  public:
    explicit Camera(glm::vec3 position, glm::vec2 angles = {DEFAULT_YAW, DEFAULT_PITCH}, float fovy = DEFAULT_FOVY,
                    float aspect_ratio = DEFAULT_ASPECT_RATIO) noexcept;

    explicit Camera(glm::vec3 position, float yaw = DEFAULT_YAW, float pitch = DEFAULT_PITCH, float fovy = DEFAULT_FOVY,
                    float aspect_ratio = DEFAULT_ASPECT_RATIO) noexcept;

    Camera() = delete;
    ~Camera();

    // disable copying
    Camera(const Camera&) = delete;
    auto operator=(const Camera&) -> Camera& = delete;

    // enable cheap moving
    Camera(Camera&&) noexcept = default;
    auto operator=(Camera&&) noexcept -> Camera& = default;

    void handleLookInput(float xoffset, float yoffset, bool constrain_pitch) noexcept;

    void setPosition(glm::vec3 position) noexcept;
    void setYaw(float yaw) noexcept;
    void setPitch(float pitch) noexcept;
    void setAspectRatio(float aspect_ratio) noexcept;
    void setLookSensitivity(float look_sensitivity) noexcept;

    [[nodiscard]] auto getPosition() const noexcept -> glm::vec3;
    [[nodiscard]] auto getYaw() const noexcept -> float;
    [[nodiscard]] auto getPitch() const noexcept -> float;
    [[nodiscard]] auto getFrontVector() const noexcept -> glm::vec3;
    [[nodiscard]] auto getUpVector() const noexcept -> glm::vec3;
    [[nodiscard]] auto getRightVector() const noexcept -> glm::vec3;
    [[nodiscard]] auto getAspectRatio() const noexcept -> float;
    [[nodiscard]] auto getLookSensitivity() const noexcept -> float;
    [[nodiscard]] auto getViewMatrix() const noexcept -> glm::mat4;
    [[nodiscard]] auto getProjectionMatrix() const noexcept -> glm::mat4;
};
