#include "engine.hpp"

#include <memory>
#include <stdexcept>

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <glad/gl.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/imgui.h>
#include <spdlog/spdlog.h>

Engine::Engine()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error("Failed to initialize SDL3");
    }
}

Engine::~Engine()
{
    SDL_Quit();
}

void Engine::initializeImGui() noexcept
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& imgui_io = ImGui::GetIO();
    imgui_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplSDL3_InitForOpenGL(this->window_->get_window_handle(), this->window_->get_context());
    ImGui_ImplOpenGL3_Init();
}

void Engine::init()
{
    this->window_ =
        std::make_unique<Window>("test window", DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);

    this->world_ = std::make_unique<World>();
    this->world_->generateArea(ChunkCoord(0, 0, 0), 10, 3, 10);

    this->world_renderer_ = std::make_unique<WorldRenderer>(this->world_.get());

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    this->window_->set_capturing_mouse(true);
    this->initializeImGui();
}

void Engine::updateFrametimes() noexcept
{
    this->frame_data_.delta_time =
        static_cast<double>((SDL_GetTicksNS() - this->frame_data_.last_time_ns)) / 1000000000.0;
    this->frame_data_.last_time_ns = SDL_GetTicksNS();
    this->frame_data_.recent_frame_times.push_back(this->frame_data_.delta_time);

    this->total_time_passed_ += this->frame_data_.delta_time;
    this->time_since_last_log_ += this->frame_data_.delta_time;

    this->window_->set_title(std::format("test window | {:.4}s", this->total_time_passed_));

    if (this->time_since_last_log_ > 5.0) {
        double average_frame_time = 0.0;
        for (auto it = this->frame_data_.recent_frame_times.begin(); it <= this->frame_data_.recent_frame_times.end();
             it++) {
            average_frame_time += *it;
        }

        average_frame_time /= static_cast<double>(this->frame_data_.recent_frame_times.size());

        spdlog::debug("{} frames rendered in 5.0 seconds. average frametime: {:.4f}, FPS: {}",
                      this->frame_data_.recent_frame_times.size(), average_frame_time, 1 / average_frame_time);
        this->time_since_last_log_ -= 5.0F;
        this->frame_data_.recent_frame_times.clear();
    }
}

void Engine::processEvents() noexcept
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Send all events to imgui, if the camera is not being controlled
        if (!this->window_->is_capturing_mouse()) {
            ImGui_ImplSDL3_ProcessEvent(&event);
        }

        // Send all window related events to the window
        if (event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST) {
            this->window_->handle_event(event.window);
        }

        if (event.type == SDL_EVENT_MOUSE_MOTION && this->window_->is_capturing_mouse()) {
            this->world_->getLocalPlayer()->getCamera()->handleLookInput(event.motion.xrel, -event.motion.yrel, true);
        }
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE) {
            this->window_->set_capturing_mouse(!this->window_->is_capturing_mouse());
        }
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            this->world_->getLocalPlayer()->getCamera()->setAspectRatio(static_cast<float>(event.window.data1) /
                                                                        static_cast<float>(event.window.data2));
        }

        if (event.type == SDL_EVENT_QUIT) {
            this->should_quit_ = true;
        }
    }
}

void Engine::prepareFrame() noexcept
{
    this->updateFrametimes();

    // prepare imgui for the framestart
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    glClearColor(1.0F, 1.0F, 1.0F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Engine::endFrame() noexcept
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(this->window_->get_window_handle());
}

void Engine::run()
{
    this->frame_data_.last_time_ns = SDL_GetTicksNS();

    while (!this->should_quit_) {
        this->prepareFrame();

        this->processEvents();

        if (this->window_->is_capturing_mouse()) {
            this->world_->getLocalPlayer()->tick(static_cast<float>(this->frame_data_.delta_time));
        }

        ImGui::Begin("Info", nullptr, 0);

        Player* player = this->world_->getLocalPlayer();
        Camera* camera = player->getCamera();
        glm::vec3 position = camera->getPosition();
        ImGui::Text("Position: X: %.4f Y: %.4f Z: %.4f", position.x, position.y, position.z);
        ImGui::Text("Orientation: Yaw: %.2f Pitch: %.2f", camera->getYaw(), camera->getPitch());
        ImGui::End();

        if (!this->window_->is_capturing_mouse()) {
            ImGui::Begin("Settings", nullptr, 0);
            ImGui::Checkbox("Wireframe", &this->checkbox_is_ticked_);

            if (this->checkbox_is_ticked_ != this->checkbox_was_ticked_) {
                if (this->checkbox_is_ticked_) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                } else {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }

                this->checkbox_was_ticked_ = this->checkbox_is_ticked_;
            }

            ImGui::End();
        }

        world_->tick();
        world_renderer_->render();

        this->endFrame();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    // needed to avoid segmentation fault when force quitting
    SDL_SetWindowRelativeMouseMode(this->window_->get_window_handle(), false);
}
