#pragma once

#include <cstdint>
#include <optional>
#include <queue>
#include <unordered_map>

#include <noise/FastNoiseLite.h>

#include "chunk.hpp"
#include "coordinates.hpp"
#include "player.hpp"

enum class WorldEventType : std::uint8_t {
    ChunkLoaded,
    ChunkUnloaded,
    ChunkDirty,
};

using WorldEvent = struct WorldEvent {
    WorldEventType type;
    std::uint64_t hash;
    ChunkCoord position;
};

class World
{
  private:
    int m_seed = 0;
    FastNoiseLite m_noise_generator;

    std::unordered_map<std::uint64_t, std::unique_ptr<Chunk>> m_loaded_chunks;
    std::queue<WorldEvent> m_events;

    std::unique_ptr<Player> m_local_player;

    [[nodiscard]] auto isChunkLoaded(ChunkCoord position) const noexcept -> bool;
    void initializeNoiseGenerator();

  public:
    explicit World(int seed = 0);
    ~World() = default;

    // disable copying
    World(const World&) = delete;
    auto operator=(const World&) -> World& = delete;

    // enable moving
    World(World&&) noexcept = delete;
    auto operator=(World&&) noexcept -> World& = delete;

    void tick();

    [[nodiscard]] auto tryGetBlock(WorldCoord position) const noexcept -> std::optional<Block>;

    auto trySetBlock(WorldCoord position, Block block) noexcept -> bool;

    [[nodiscard]] auto tryGetChunk(ChunkCoord position) const -> Chunk*;

    [[nodiscard]] auto getSeed() const noexcept -> int;
    void setSeed(int seed) noexcept;

    auto getLocalPlayer() noexcept -> Player*;

    auto getEvents() -> std::queue<WorldEvent>&;

    auto generateChunk(ChunkCoord position) -> Chunk&;
    void generateArea(ChunkCoord center, int x_radius, int y_radius, int z_radius);
};
