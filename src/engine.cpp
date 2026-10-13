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

    ImGui_ImplSDL3_InitForOpenGL(this->m_window->get_window_handle(), this->m_window->get_context());
    ImGui_ImplOpenGL3_Init();
}

void Engine::init()
{
    this->m_window =
        std::make_unique<Window>("test window", DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);

    this->m_world = std::make_unique<World>();
    this->m_world->generateArea(ChunkCoord(0, 0, 0), 10, 3, 10);

    this->m_world_renderer = std::make_unique<WorldRenderer>(this->m_world.get());

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    this->m_window->set_capturing_mouse(true);
    this->initializeImGui();
}

void Engine::updateFrametimes() noexcept
{
    this->m_frame_data.delta_time =
        static_cast<double>((SDL_GetTicksNS() - this->m_frame_data.last_time_ns)) / 1000000000.0;
    this->m_frame_data.last_time_ns = SDL_GetTicksNS();
    this->m_frame_data.recent_frame_times.push_back(this->m_frame_data.delta_time);

    this->m_total_time_passed += this->m_frame_data.delta_time;
    this->m_time_since_last_log += this->m_frame_data.delta_time;

    this->m_window->set_title(std::format("test window | {:.4}s", this->m_total_time_passed));

    if (this->m_time_since_last_log > 5.0) {
        double average_frame_time = 0.0;
        for (auto it = this->m_frame_data.recent_frame_times.begin(); it <= this->m_frame_data.recent_frame_times.end();
             it++) {
            average_frame_time += *it;
        }

        average_frame_time /= static_cast<double>(this->m_frame_data.recent_frame_times.size());

        spdlog::debug("{} frames rendered in 5.0 seconds. average frametime: {:.4f}, FPS: {}",
                      this->m_frame_data.recent_frame_times.size(), average_frame_time, 1 / average_frame_time);
        this->m_time_since_last_log -= 5.0F;
        this->m_frame_data.recent_frame_times.clear();
    }
}

void Engine::processEvents() noexcept
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Send all events to imgui, if the camera is not being controlled
        if (!this->m_window->is_capturing_mouse()) {
            ImGui_ImplSDL3_ProcessEvent(&event);
        }

        // Send all window related events to the window
        if (event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST) {
            this->m_window->handle_event(event.window);
        }

        if (event.type == SDL_EVENT_MOUSE_MOTION && this->m_window->is_capturing_mouse()) {
            this->m_world->getLocalPlayer()->getCamera()->handleLookInput(event.motion.xrel, -event.motion.yrel, true);
        }
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE) {
            this->m_window->set_capturing_mouse(!this->m_window->is_capturing_mouse());
        }
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            this->m_world->getLocalPlayer()->getCamera()->setAspectRatio(static_cast<float>(event.window.data1) /
                                                                         static_cast<float>(event.window.data2));
        }

        if (event.type == SDL_EVENT_QUIT) {
            this->m_should_quit = true;
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

    SDL_GL_SwapWindow(this->m_window->get_window_handle());
}

void Engine::run()
{
    this->m_frame_data.last_time_ns = SDL_GetTicksNS();

    while (!this->m_should_quit) {
        this->prepareFrame();

        this->processEvents();

        if (this->m_window->is_capturing_mouse()) {
            this->m_world->getLocalPlayer()->tick(static_cast<float>(this->m_frame_data.delta_time));
        }

        ImGui::Begin("Info", nullptr, 0);

        Player* player = this->m_world->getLocalPlayer();
        Camera* camera = player->getCamera();
        glm::vec3 position = camera->getPosition();
        ImGui::Text("Position: X: %.4f Y: %.4f Z: %.4f", position.x, position.y, position.z);
        ImGui::Text("Orientation: Yaw: %.2f Pitch: %.2f", camera->getYaw(), camera->getPitch());
        ImGui::End();

        if (!this->m_window->is_capturing_mouse()) {
            ImGui::Begin("Settings", nullptr, 0);
            ImGui::Checkbox("Wireframe", &this->m_checkbox_is_ticked);

            if (this->m_checkbox_is_ticked != this->m_checkbox_was_ticked) {
                if (this->m_checkbox_is_ticked) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                } else {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }

                this->m_checkbox_was_ticked = this->m_checkbox_is_ticked;
            }

            ImGui::End();
        }

        m_world->tick();
        m_world_renderer->render();

        this->endFrame();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    // needed to avoid segmentation fault when force quitting
    SDL_SetWindowRelativeMouseMode(this->m_window->get_window_handle(), false);
}
