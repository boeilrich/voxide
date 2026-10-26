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
    std::unique_ptr<Window> window_;
    std::unique_ptr<World> world_;

    std::unique_ptr<WorldRenderer> world_renderer_;

    FrameData frame_data_;
    double total_time_passed_ = 0.0F;
    double time_since_last_log_ = 0.0F;

    bool checkbox_is_ticked_ = false;
    bool checkbox_was_ticked_ = false;

    bool should_quit_ = false;

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
