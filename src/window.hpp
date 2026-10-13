#pragma once

#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

class Window
{
  private:
    SDL_Window* m_window_handle = nullptr;
    SDL_GLContext m_context = nullptr;

    bool m_visible = false;
    bool m_resizable = false;
    bool m_fullscreen = false;
    bool m_always_on_top = false;
    bool m_capturing_mouse = false;

    int m_vsync_mode = 1;

    std::pair<int, int> m_position;
    std::pair<int, int> m_size;

    std::string m_title;

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
