#include "window.hpp"

#include <optional>
#include <stdexcept>

#include <spdlog/spdlog.h>

Window::Window(const std::string& title, int width, int height, SDL_WindowFlags flags, bool create_gl_context,
               const GLContextInfo& info)
{
    // setup the opengl context arguments and set flag if opengl context is requested by caller
    if (create_gl_context) {
        this->setupGLContextParameters(info);
        flags |= SDL_WINDOW_OPENGL;
    }

    handle_ = SDL_CreateWindow(title.c_str(), width, height, flags);

    if (!handle_) {
        throw std::runtime_error(SDL_GetError());
    }

    if (!create_gl_context) {
        return;
    }

    context_ = SDL_GL_CreateContext(handle_);

    if (!context_) {
        SDL_DestroyWindow(handle_);
        throw std::runtime_error(SDL_GetError());
    }

    this->populateGLContextInfo();
}

Window::~Window()
{
    if (context_) {
        if (SDL_GL_GetCurrentContext() == context_) {
            SDL_GL_MakeCurrent(nullptr, nullptr);
        }
        SDL_GL_DestroyContext(context_);
    }
    if (handle_) {
        SDL_DestroyWindow(handle_);
    }
}

Window::Window(Window&& other) noexcept
    : handle_(other.handle_),
      context_(other.context_),
      context_info_(other.context_info_)
{
    other.handle_ = nullptr;
    other.context_ = nullptr;
}

auto Window::operator=(Window&& other) noexcept -> Window&
{
    if (this != &other) {
        if (context_) {
            SDL_GL_DestroyContext(context_);
        }
        if (handle_) {
            SDL_DestroyWindow(handle_);
        }

        handle_ = other.handle_;
        context_ = other.context_;
        context_info_ = other.context_info_;

        other.handle_ = nullptr;
        other.context_ = nullptr;
    }
    return *this;
}

auto Window::swapBuffers() -> bool
{
    if (!context_) {
        spdlog::warn("swapBuffers() called on window without gl context");
        return false;
    }

    if (!SDL_GL_SwapWindow(handle_)) {
        spdlog::error("failed to swap buffers: {}", SDL_GetError());
        return false;
    }

    return true;
}

auto Window::makeContextCurrent() -> bool
{
    if (!context_) {
        spdlog::warn("makeContextCurrent() called on window with no gl context");
        return false;
    }

    return SDL_GL_MakeCurrent(handle_, context_);
}

auto Window::setVisible(bool visible) noexcept -> bool
{
    if (visible) {
        return SDL_ShowWindow(handle_);
    } else {
        return SDL_HideWindow(handle_);
    }
}

auto Window::setTitle(const std::string& title) noexcept -> bool
{
    return SDL_SetWindowTitle(handle_, title.c_str());
}

auto Window::setSize(int width, int height) noexcept -> bool
{
    return SDL_SetWindowSize(handle_, width, height);
}

auto Window::setPosition(int xpos, int ypos) noexcept -> bool
{
    return SDL_SetWindowPosition(handle_, xpos, ypos);
}

auto Window::setResizable(bool resizable) noexcept -> bool
{
    return SDL_SetWindowResizable(handle_, resizable);
}

auto Window::setFullscreen(bool fullscreen) noexcept -> bool
{
    return SDL_SetWindowFullscreen(handle_, fullscreen);
}

auto Window::setAlwaysOnTop(bool always_on_top) noexcept -> bool
{
    return SDL_SetWindowAlwaysOnTop(handle_, always_on_top);
}

auto Window::setRelativeMouseMode(bool relative_mouse_mode) noexcept -> bool
{
    return SDL_SetWindowRelativeMouseMode(handle_, relative_mouse_mode);
}

auto Window::setVSync(int swap_interval) noexcept -> bool
{
    if (!context_) {
        return false;
    }

    this->storeContext();

    if (!this->makeContextCurrent()) {
        this->restoreContext();
        return false;
    }

    if (!SDL_GL_SetSwapInterval(swap_interval)) {
        spdlog::error("failed to set vsync state: {}", SDL_GetError());
        this->restoreContext();
        return false;
    }

    this->restoreContext();

    return true;
}

auto Window::getTitle() const -> std::string
{
    return SDL_GetWindowTitle(handle_);
}

auto Window::getSize() const noexcept -> std::pair<int, int>
{
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(handle_, &width, &height);
    return {width, height};
}

auto Window::getWidth() const noexcept -> int
{
    int width = 0;
    SDL_GetWindowSize(handle_, &width, nullptr);
    return width;
}

auto Window::getHeight() const noexcept -> int
{
    int height = 0;
    SDL_GetWindowSize(handle_, nullptr, &height);
    return height;
}

auto Window::getAspectRatio() const noexcept -> float
{
    std::pair<int, int> size = this->getSize();

    // protect against 0 height
    if (size.second == 0) {
        return 0;
    }

    return static_cast<float>(size.first) / static_cast<float>(size.second);
}

auto Window::getPosition() const noexcept -> std::pair<int, int>
{
    int xpos = 0;
    int ypos = 0;
    SDL_GetWindowPosition(handle_, &xpos, &ypos);
    return {xpos, ypos};
}

auto Window::getXPos() const noexcept -> int
{
    int xpos = 0;
    SDL_GetWindowPosition(handle_, &xpos, nullptr);
    return xpos;
}

auto Window::getYPos() const noexcept -> int
{
    int ypos = 0;
    SDL_GetWindowPosition(handle_, nullptr, &ypos);
    return ypos;
}

auto Window::isVisible() const noexcept -> bool
{
    return !(SDL_GetWindowFlags(handle_) & SDL_WINDOW_HIDDEN);
}

auto Window::isResizable() const noexcept -> bool
{
    return SDL_GetWindowFlags(handle_) & SDL_WINDOW_RESIZABLE;
}

auto Window::isFullscreen() const noexcept -> bool
{
    return SDL_GetWindowFlags(handle_) & SDL_WINDOW_FULLSCREEN;
}

auto Window::isAlwaysOnTop() const noexcept -> bool
{
    return SDL_GetWindowFlags(handle_) & SDL_WINDOW_ALWAYS_ON_TOP;
}

auto Window::getRelativeMouseMode() const noexcept -> bool
{
    return SDL_GetWindowFlags(handle_) & SDL_WINDOW_MOUSE_RELATIVE_MODE;
}

auto Window::getVSync() noexcept -> std::optional<int>
{
    if (!context_) {
        return std::nullopt;
    }

    this->storeContext();

    if (!this->makeContextCurrent()) {
        this->restoreContext();
        return std::nullopt;
    }

    int swap_interval = 0;
    if (!SDL_GL_GetSwapInterval(&swap_interval)) {
        spdlog::error("failed to get vsync state: {}", SDL_GetError());
        this->restoreContext();
        return std::nullopt;
    }

    this->restoreContext();
    return swap_interval;
}

auto Window::getHandle() const noexcept -> SDL_Window*
{
    return handle_;
}

auto Window::getContext() const noexcept -> SDL_GLContext
{
    return context_;
}

void Window::storeContext()
{
    store_.context_ = SDL_GL_GetCurrentContext();
    store_.handle_ = SDL_GL_GetCurrentWindow();
}

void Window::restoreContext()
{
    if (!SDL_GL_MakeCurrent(store_.handle_, store_.context_)) {
        spdlog::warn("failed to restore context: {}", SDL_GetError());
    }
}

void Window::populateGLContextInfo()
{
    GLContextInfo info{};

    SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &info.color_buffer.red_size);
    SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &info.color_buffer.green_size);
    SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &info.color_buffer.blue_size);
    SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &info.color_buffer.alpha_size);
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &info.depth_buffer_size);
    SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &info.stencil_buffer_size);

    int doublebuffering = 0;
    SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doublebuffering);
    info.doublebuffering = doublebuffering > 0;

    int hardware_acceleration = 0;
    SDL_GL_GetAttribute(SDL_GL_ACCELERATED_VISUAL, &hardware_acceleration);
    info.hardware_acceleration = hardware_acceleration > 0;

    int multisampling = 0;
    SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &multisampling);
    info.multisampling.enabled = multisampling > 0;
    if (info.multisampling.enabled) {
        SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &info.multisampling.samples);
    }

    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &info.version.major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &info.version.minor);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &info.version.profile_mask);

    int flags = 0;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &flags);
    info.version.debug_context = (flags & SDL_GL_CONTEXT_DEBUG_FLAG) > 0;
    info.version.forward_compatible = (flags & SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG) > 0;

    context_info_ = info;
    spdlog::debug(
        "got OpenGL context: {}.{} (profile={}, debug={}, forward_compat={})\n"
        "  - color: {}/{}/{}/{}\n"
        "  - depth: {}\n"
        "  - stencil: {}\n"
        "  - doublebuffer: {}\n"
        "  - hw_accel: {}\n"
        "  - msaa: {} x{}",
        info.version.major, info.version.minor, info.version.profile_mask, info.version.debug_context,
        info.version.forward_compatible, info.color_buffer.red_size, info.color_buffer.green_size,
        info.color_buffer.blue_size, info.color_buffer.alpha_size, info.depth_buffer_size, info.stencil_buffer_size,
        info.doublebuffering, info.hardware_acceleration, info.multisampling.enabled, info.multisampling.samples);
}

void Window::setupGLContextParameters(const GLContextInfo& info)
{
    spdlog::debug(
        "requesting OpenGL context: {}.{} (profile={}, debug={}, forward_compat={})\n"
        "  - color: {}/{}/{}/{}\n"
        "  - depth: {}\n"
        "  - stencil: {}\n"
        "  - doublebuffer: {}\n"
        "  - hw_accel: {}\n"
        "  - msaa: {} x{}",
        info.version.major, info.version.minor, info.version.profile_mask, info.version.debug_context,
        info.version.forward_compatible, info.color_buffer.red_size, info.color_buffer.green_size,
        info.color_buffer.blue_size, info.color_buffer.alpha_size, info.depth_buffer_size, info.stencil_buffer_size,
        info.doublebuffering, info.hardware_acceleration, info.multisampling.enabled, info.multisampling.samples);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, info.color_buffer.red_size);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, info.color_buffer.green_size);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, info.color_buffer.blue_size);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, info.color_buffer.alpha_size);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, info.depth_buffer_size);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, info.stencil_buffer_size);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, info.doublebuffering ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, info.hardware_acceleration ? 1 : 0);

    if (info.multisampling.enabled) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, info.multisampling.samples);
    } else {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, info.version.major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, info.version.minor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, info.version.profile_mask);

    int flags = 0;
    if (info.version.forward_compatible)
        flags |= SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
    if (info.version.debug_context)
        flags |= SDL_GL_CONTEXT_DEBUG_FLAG;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, flags);
}
