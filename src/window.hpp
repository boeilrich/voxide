#pragma once

#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

class Window
{
  private:
    SDL_Window* window_handle_ = nullptr;
    SDL_GLContext context_ = nullptr;

    bool visible_ = false;
    bool resizeable_ = false;
    bool fullscreen_ = false;
    bool always_on_top_ = false;
    bool capturing_mouse_ = false;

    int vsync_mode_ = 1;

    std::pair<int, int> position_;
    std::pair<int, int> size_;

    std::string title_;

    auto create_context() -> bool;

  public:
    Window(const std::string& title, int width, int height, SDL_WindowFlags flags);
    ~Window();

    // disable copying
    Window(const Window&) = delete;
    auto operator=(const Window&) -> Window& = delete;

    // enable moving
    Window(Window&&) noexcept = default;
    auto operator=(Window&&) noexcept -> Window& = default;

    [[nodiscard]] auto get_window_handle() const noexcept -> SDL_Window*;
    [[nodiscard]] auto get_context() const noexcept -> SDL_GLContext;

    void handle_event(SDL_WindowEvent event) noexcept;

    void set_visible(bool visible) noexcept;
    void set_title(const std::string& title) noexcept;
    void set_size(int width, int height) noexcept;
    void set_position(int xpos, int ypos) noexcept;
    void set_resizable(bool resizable) noexcept;
    void set_fullscreen(bool fullscreen) noexcept;
    void set_always_on_top(bool always_on_top) noexcept;
    void set_capturing_mouse(bool capturing_mouse) noexcept;
    void set_vsync_mode(int mode) noexcept;

    [[nodiscard]] auto get_title() const noexcept -> std::string;
    [[nodiscard]] auto get_size() const noexcept -> std::pair<int, int>;
    [[nodiscard]] auto get_width() const noexcept -> int;
    [[nodiscard]] auto get_height() const noexcept -> int;

    [[nodiscard]] auto get_position() const noexcept -> std::pair<int, int>;
    [[nodiscard]] auto get_xpos() const noexcept -> int;
    [[nodiscard]] auto get_ypos() const noexcept -> int;

    [[nodiscard]] auto is_visible() const noexcept -> bool;
    [[nodiscard]] auto is_resizable() const noexcept -> bool;
    [[nodiscard]] auto is_fullscreen() const noexcept -> bool;
    [[nodiscard]] auto is_always_on_top() const noexcept -> bool;
    [[nodiscard]] auto is_capturing_mouse() const noexcept -> bool;

    [[nodiscard]] auto get_vsync_mode() const noexcept -> int;
};
