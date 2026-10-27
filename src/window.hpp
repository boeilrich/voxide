#include <optional>
#include <string>

#include <SDL3/SDL.h>

struct ColorBufferInfo {
    int red_size = 8;
    int green_size = 8;
    int blue_size = 8;
    int alpha_size = 8;
};

struct MultisampleInfo {
    bool enabled = false;
    int samples = 0;
};

struct GLContextVersionInfo {
    int major = 3;
    int minor = 3;
    int profile_mask = SDL_GL_CONTEXT_PROFILE_CORE;
    bool forward_compatible = true;
    bool debug_context = false;
};

struct GLContextInfo {
    ColorBufferInfo color_buffer{};
    bool doublebuffering = true;
    int depth_buffer_size = 24;
    int stencil_buffer_size = 8;
    MultisampleInfo multisampling{};
    bool hardware_acceleration = true;
    GLContextVersionInfo version{};
};

class Window
{
  public:
    Window(const std::string& title, int width, int height, SDL_WindowFlags flags, bool create_gl_context,
           const GLContextInfo& info = GLContextInfo{});
    ~Window();

    // disable copying
    Window(const Window&) = delete;
    auto operator=(const Window&) -> Window& = delete;

    // enable moving
    Window(Window&& other) noexcept;
    auto operator=(Window&& other) noexcept -> Window&;

    auto swapBuffers() -> bool;
    auto makeContextCurrent() -> bool;

    auto setVisible(bool visible) noexcept -> bool;
    auto setTitle(const std::string& title) noexcept -> bool;
    auto setSize(int width, int height) noexcept -> bool;
    auto setPosition(int xpos, int ypos) noexcept -> bool;
    auto setResizable(bool resizable) noexcept -> bool;
    auto setFullscreen(bool fullscreen) noexcept -> bool;
    auto setAlwaysOnTop(bool always_on_top) noexcept -> bool;
    auto setRelativeMouseMode(bool relative_mouse_mode) noexcept -> bool;
    auto setVSync(int swap_interval) noexcept -> bool;

    [[nodiscard]] auto getTitle() const -> std::string;
    [[nodiscard]] auto getSize() const noexcept -> std::pair<int, int>;
    [[nodiscard]] auto getWidth() const noexcept -> int;
    [[nodiscard]] auto getHeight() const noexcept -> int;
    [[nodiscard]] auto getAspectRatio() const noexcept -> float;

    [[nodiscard]] auto getPosition() const noexcept -> std::pair<int, int>;
    [[nodiscard]] auto getXPos() const noexcept -> int;
    [[nodiscard]] auto getYPos() const noexcept -> int;

    [[nodiscard]] auto isVisible() const noexcept -> bool;
    [[nodiscard]] auto isResizable() const noexcept -> bool;
    [[nodiscard]] auto isFullscreen() const noexcept -> bool;
    [[nodiscard]] auto isAlwaysOnTop() const noexcept -> bool;

    [[nodiscard]] auto getRelativeMouseMode() const noexcept -> bool;

    [[nodiscard]] auto getVSync() noexcept -> std::optional<int>;

    [[nodiscard]] auto getHandle() const noexcept -> SDL_Window*;
    [[nodiscard]] auto getContext() const noexcept -> SDL_GLContext;

  private:
    SDL_Window* handle_ = nullptr;
    SDL_GLContext context_ = nullptr;
    std::optional<GLContextInfo> context_info_;

    struct store_ {
        SDL_Window* handle_ = nullptr;
        SDL_GLContext context_ = nullptr;
    } store_;

    static void setupGLContextParameters(const GLContextInfo& info);
    void storeContext();
    void restoreContext();
    void populateGLContextInfo();
};
