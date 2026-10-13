#include "window.hpp"

#include <stdexcept>
#include <utility>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_video.h>
#include <glad/gl.h>
#include <spdlog/spdlog.h>

constexpr int DEFAULT_WIDTH = 1024;
constexpr int DEFAULT_HEIGHT = 768;

auto Window::create_context() -> bool
{
    // set OpenGL version
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    // "enforce" modern opengl, required on macOS
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    // set core profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // request multisample buffer with 8x
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    // enable doublebuffering, required for vsync
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // 24-bit depth buffer
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GLContext context = SDL_GL_CreateContext(this->m_window_handle);
    if (context == nullptr) {
        spdlog::error("Failed to create SDL3 OpenGL context: {}", SDL_GetError());
        return false;
    }

    SDL_GL_MakeCurrent(this->m_window_handle, context);
    if (!SDL_GL_SetSwapInterval(0)) {
        spdlog::warn("Failed to enable VSync: {}", SDL_GetError());
    }

    this->m_context = context;

    int minor = 0;
    int major = 0;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

    spdlog::info("OpenGL {}.{} Context!", major, minor);

    return true;
}

Window::Window(const std::string& title, int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT,
               SDL_WindowFlags flags = 0)
{
    spdlog::trace("Creating Window ('{}') w={}, h={}, flags={}", title, width, height, flags);

    this->m_visible = (flags & SDL_WINDOW_HIDDEN) == 0;
    this->m_resizable = (flags & SDL_WINDOW_RESIZABLE) != 0;
    this->m_fullscreen = (flags & SDL_WINDOW_FULLSCREEN) != 0;
    this->m_always_on_top = (flags & SDL_WINDOW_ALWAYS_ON_TOP) != 0;
    this->m_capturing_mouse = false;

    this->m_window_handle = SDL_CreateWindow(title.c_str(), width, height, flags | SDL_WINDOW_OPENGL);
    if (this->m_window_handle == nullptr) {
        spdlog::error("Failed to create SDL3 Window!");
        throw std::runtime_error("Failed to create SDL3 Window!");
    }

    if (!this->create_context()) {
        throw std::runtime_error("Failed to create SDL3 OpenGL context!");
    }

    int version = gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress));
    if (version == 0) {
        spdlog::error("Failed to initialize GLAD!");
        throw std::runtime_error("Failed to initialize GLAD!");
    }
    spdlog::info("OpenGL {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    glViewport(0, 0, width, height);
}

Window::~Window()
{
    spdlog::trace("Destroying Window {}", fmt::ptr(this->m_window_handle));

    assert(this->m_context != nullptr);
    SDL_GL_DestroyContext(this->m_context);

    assert(this->m_window_handle != nullptr);
    SDL_DestroyWindow(this->m_window_handle);
}

[[nodiscard]] auto Window::get_window_handle() const noexcept -> SDL_Window*
{
    return this->m_window_handle;
}

[[nodiscard]] auto Window::get_context() const noexcept -> SDL_GLContext
{
    return this->m_context;
}

void Window::handle_event(SDL_WindowEvent event) noexcept
{
    switch (event.type) {
        case SDL_EVENT_WINDOW_SHOWN:
            this->m_visible = true;
            spdlog::trace("Setting window visibility to {}", this->m_visible);
            break;
        case SDL_EVENT_WINDOW_HIDDEN:
            this->m_visible = false;
            spdlog::trace("Setting window visibility to {}", this->m_visible);
            break;
        case SDL_EVENT_WINDOW_MOVED:
            this->m_position = {event.data1, event.data2};
            // spdlog::trace("Window moved to x={}, y={}", event.data1, event.data2);
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            this->m_size = {event.data1, event.data2};
            glViewport(0, 0, event.data1, event.data2);
            // spdlog::trace("Window resized to w={}, h={}", event.data1, event.data2);
            break;
        case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
            this->m_fullscreen = true;
            spdlog::trace("Setting window fullscreen to {}", this->m_fullscreen);
            break;
        case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
            this->m_fullscreen = false;
            spdlog::trace("Setting window fullscreen to {}", this->m_fullscreen);
            break;
        default:
            break;
    }
}

void Window::set_visible(bool visible) noexcept
{
    if (visible) {
        SDL_ShowWindow(this->m_window_handle);
    } else {
        SDL_HideWindow(this->m_window_handle);
    }
}

void Window::set_title(const std::string& title) noexcept
{
    SDL_SetWindowTitle(this->m_window_handle, title.c_str());
    this->m_title = title;
}

void Window::set_size(int width, int height) noexcept
{
    SDL_SetWindowSize(this->m_window_handle, width, height);
}

void Window::set_position(int xpos, int ypos) noexcept
{
    SDL_SetWindowPosition(this->m_window_handle, xpos, ypos);
}

void Window::set_resizable(bool resizable) noexcept
{
    SDL_SetWindowResizable(this->m_window_handle, resizable);
    this->m_resizable = resizable;
}

void Window::set_fullscreen(bool fullscreen) noexcept
{
    SDL_SetWindowFullscreen(this->m_window_handle, fullscreen);
}

void Window::set_always_on_top(bool always_on_top) noexcept
{
    SDL_SetWindowAlwaysOnTop(this->m_window_handle, always_on_top);
    this->m_always_on_top = always_on_top;
}

void Window::set_capturing_mouse(bool capturing_mouse) noexcept
{
    SDL_SetWindowRelativeMouseMode(this->m_window_handle, capturing_mouse);
    this->m_capturing_mouse = capturing_mouse;
}

// mode: 0 = disabled, 1 = enabled, -1 = adaptive
void Window::set_vsync_mode(int mode) noexcept
{
    SDL_GL_MakeCurrent(this->m_window_handle, this->m_context);

    if (!SDL_GL_SetSwapInterval(mode)) {
        spdlog::warn("Failed to change VSync mode: {}", SDL_GetError());
        return;
    }

    this->m_vsync_mode = mode;
}

auto Window::get_title() const noexcept -> std::string
{
    const char* title = SDL_GetWindowTitle(this->m_window_handle);
    return {title};
}

auto Window::get_size() const noexcept -> std::pair<int, int>
{
    return this->m_size;
}

auto Window::get_width() const noexcept -> int
{
    return this->m_size.first;
}

auto Window::get_height() const noexcept -> int
{
    return this->m_size.second;
}

auto Window::get_position() const noexcept -> std::pair<int, int>
{
    return this->m_position;
}

auto Window::get_xpos() const noexcept -> int
{
    return this->m_position.first;
}

auto Window::get_ypos() const noexcept -> int
{
    return this->m_position.second;
}

auto Window::is_visible() const noexcept -> bool
{
    return this->m_visible;
}

auto Window::is_resizable() const noexcept -> bool
{
    return this->m_resizable;
}

auto Window::is_fullscreen() const noexcept -> bool
{
    return this->m_fullscreen;
}

auto Window::is_always_on_top() const noexcept -> bool
{
    return this->m_always_on_top;
}

auto Window::is_capturing_mouse() const noexcept -> bool
{
    return this->m_capturing_mouse;
}

auto Window::get_vsync_mode() const noexcept -> int
{
    return this->m_vsync_mode;
}
