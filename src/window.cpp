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

    SDL_GLContext context = SDL_GL_CreateContext(this->window_handle_);
    if (context == nullptr) {
        spdlog::error("Failed to create SDL3 OpenGL context: {}", SDL_GetError());
        return false;
    }

    SDL_GL_MakeCurrent(this->window_handle_, context);
    if (!SDL_GL_SetSwapInterval(0)) {
        spdlog::warn("Failed to enable VSync: {}", SDL_GetError());
    }

    this->context_ = context;

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

    this->visible_ = (flags & SDL_WINDOW_HIDDEN) == 0;
    this->resizeable_ = (flags & SDL_WINDOW_RESIZABLE) != 0;
    this->fullscreen_ = (flags & SDL_WINDOW_FULLSCREEN) != 0;
    this->always_on_top_ = (flags & SDL_WINDOW_ALWAYS_ON_TOP) != 0;
    this->capturing_mouse_ = false;

    this->window_handle_ = SDL_CreateWindow(title.c_str(), width, height, flags | SDL_WINDOW_OPENGL);
    if (this->window_handle_ == nullptr) {
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
    spdlog::trace("Destroying Window {}", fmt::ptr(this->window_handle_));

    assert(this->context_ != nullptr);
    SDL_GL_DestroyContext(this->context_);

    assert(this->window_handle_ != nullptr);
    SDL_DestroyWindow(this->window_handle_);
}

[[nodiscard]] auto Window::get_window_handle() const noexcept -> SDL_Window*
{
    return this->window_handle_;
}

[[nodiscard]] auto Window::get_context() const noexcept -> SDL_GLContext
{
    return this->context_;
}

void Window::handle_event(SDL_WindowEvent event) noexcept
{
    switch (event.type) {
        case SDL_EVENT_WINDOW_SHOWN:
            this->visible_ = true;
            spdlog::trace("Setting window visibility to {}", this->visible_);
            break;
        case SDL_EVENT_WINDOW_HIDDEN:
            this->visible_ = false;
            spdlog::trace("Setting window visibility to {}", this->visible_);
            break;
        case SDL_EVENT_WINDOW_MOVED:
            this->position_ = {event.data1, event.data2};
            // spdlog::trace("Window moved to x={}, y={}", event.data1, event.data2);
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            this->size_ = {event.data1, event.data2};
            glViewport(0, 0, event.data1, event.data2);
            // spdlog::trace("Window resized to w={}, h={}", event.data1, event.data2);
            break;
        case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
            this->fullscreen_ = true;
            spdlog::trace("Setting window fullscreen to {}", this->fullscreen_);
            break;
        case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
            this->fullscreen_ = false;
            spdlog::trace("Setting window fullscreen to {}", this->fullscreen_);
            break;
        default:
            break;
    }
}

void Window::set_visible(bool visible) noexcept
{
    if (visible) {
        SDL_ShowWindow(this->window_handle_);
    } else {
        SDL_HideWindow(this->window_handle_);
    }
}

void Window::set_title(const std::string& title) noexcept
{
    SDL_SetWindowTitle(this->window_handle_, title.c_str());
    this->title_ = title;
}

void Window::set_size(int width, int height) noexcept
{
    SDL_SetWindowSize(this->window_handle_, width, height);
}

void Window::set_position(int xpos, int ypos) noexcept
{
    SDL_SetWindowPosition(this->window_handle_, xpos, ypos);
}

void Window::set_resizable(bool resizable) noexcept
{
    SDL_SetWindowResizable(this->window_handle_, resizable);
    this->resizeable_ = resizable;
}

void Window::set_fullscreen(bool fullscreen) noexcept
{
    SDL_SetWindowFullscreen(this->window_handle_, fullscreen);
}

void Window::set_always_on_top(bool always_on_top) noexcept
{
    SDL_SetWindowAlwaysOnTop(this->window_handle_, always_on_top);
    this->always_on_top_ = always_on_top;
}

void Window::set_capturing_mouse(bool capturing_mouse) noexcept
{
    SDL_SetWindowRelativeMouseMode(this->window_handle_, capturing_mouse);
    this->capturing_mouse_ = capturing_mouse;
}

// mode: 0 = disabled, 1 = enabled, -1 = adaptive
void Window::set_vsync_mode(int mode) noexcept
{
    SDL_GL_MakeCurrent(this->window_handle_, this->context_);

    if (!SDL_GL_SetSwapInterval(mode)) {
        spdlog::warn("Failed to change VSync mode: {}", SDL_GetError());
        return;
    }

    this->vsync_mode_ = mode;
}

auto Window::get_title() const noexcept -> std::string
{
    const char* title = SDL_GetWindowTitle(this->window_handle_);
    return {title};
}

auto Window::get_size() const noexcept -> std::pair<int, int>
{
    return this->size_;
}

auto Window::get_width() const noexcept -> int
{
    return this->size_.first;
}

auto Window::get_height() const noexcept -> int
{
    return this->size_.second;
}

auto Window::get_position() const noexcept -> std::pair<int, int>
{
    return this->position_;
}

auto Window::get_xpos() const noexcept -> int
{
    return this->position_.first;
}

auto Window::get_ypos() const noexcept -> int
{
    return this->position_.second;
}

auto Window::is_visible() const noexcept -> bool
{
    return this->visible_;
}

auto Window::is_resizable() const noexcept -> bool
{
    return this->resizeable_;
}

auto Window::is_fullscreen() const noexcept -> bool
{
    return this->fullscreen_;
}

auto Window::is_always_on_top() const noexcept -> bool
{
    return this->always_on_top_;
}

auto Window::is_capturing_mouse() const noexcept -> bool
{
    return this->capturing_mouse_;
}

auto Window::get_vsync_mode() const noexcept -> int
{
    return this->vsync_mode_;
}
