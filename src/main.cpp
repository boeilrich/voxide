#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include "engine.hpp"

auto main() -> int
{
    spdlog::set_level(spdlog::level::trace);
    spdlog::info("Starting...");

    auto* engine = new Engine();

    engine->init();
    engine->run();

    delete engine;

    spdlog::shutdown();

    return 0;
}
