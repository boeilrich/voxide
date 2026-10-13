#pragma once

#include <memory>
#include <vector>

#include <stb/stb_image.h>

#include "window.hpp"
#include "world.hpp"
#include "world_renderer.hpp"

constexpr int DEFAULT_WINDOW_WIDTH = 1920;
constexpr int DEFAULT_WINDOW_HEIGHT = 1080;

using FrameData = struct FrameData {
    double delta_time;
    unsigned long last_time_ns;
    std::vector<double> recent_frame_times;
};

class Engine
{
  private:
    std::unique_ptr<Window> m_window;
    std::unique_ptr<World> m_world;

    std::unique_ptr<WorldRenderer> m_world_renderer;

    FrameData m_frame_data;
    double m_total_time_passed = 0.0F;
    double m_time_since_last_log = 0.0F;

    bool m_checkbox_is_ticked = false;
    bool m_checkbox_was_ticked = false;

    bool m_should_quit = false;

    void initializeImGui() noexcept;

    void prepareFrame() noexcept;
    void endFrame() noexcept;

    void updateFrametimes() noexcept;

    void processEvents() noexcept;

  public:
    Engine();
    ~Engine();

    // disable copying
    Engine(const Engine&) = delete;
    auto operator=(const Engine&) -> Engine& = delete;

    // disable moving
    Engine(const Engine&&) = delete;
    auto operator=(const Engine&&) -> Engine& = delete;

    void run();
    void init();
};
