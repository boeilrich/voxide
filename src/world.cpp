#include "world.hpp"

#include <cstdint>
#include <memory>

#include <spdlog/spdlog.h>

#include "chunk.hpp"
#include "coordinates.hpp"
#include "player.hpp"

World::World(int seed) : seed_(seed)
{
    local_player_ = std::make_unique<Player>(true);
    initializeNoiseGenerator();
}

void World::initializeNoiseGenerator()
{
    noise_generator_.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise_generator_.SetFrequency(0.02F);
    noise_generator_.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise_generator_.SetFractalOctaves(3);
    noise_generator_.SetFractalLacunarity(2.0F);
    noise_generator_.SetFractalGain(0.7F);

    noise_generator_.SetSeed(seed_);
}

auto World::isChunkLoaded(ChunkCoord position) const noexcept -> bool
{
    std::uint64_t hash = Chunk::calculateHash(position);
    return loaded_chunks_.contains(hash);
}

void World::tick()
{
    for (const auto& entry : loaded_chunks_) {
        if (entry.second->isDirty()) {
            events_.emplace(WorldEvent{WorldEventType::ChunkDirty, entry.first, entry.second->getPosition()});
            entry.second->setDirty(false);
        }
    }
}

[[nodiscard]] auto World::tryGetBlock(WorldCoord position) const noexcept -> std::optional<Block>
{
    ChunkCoord chunk_position = Coords::world_to_chunk(position);
    if (!isChunkLoaded(chunk_position)) {
        return std::nullopt;
    }

    std::uint64_t hash = Chunk::calculateHash(Coords::world_to_chunk(position));
    const auto& chunk = loaded_chunks_.at(hash);

    LocalCoord local_position = Coords::world_to_local(position);
    return chunk->getBlockAt(local_position);
}

auto World::trySetBlock(WorldCoord position, Block block) noexcept -> bool
{
    ChunkCoord chunk_position = Coords::world_to_chunk(position);
    if (!isChunkLoaded(chunk_position)) {
        return false;
    }

    std::uint64_t hash = Chunk::calculateHash(Coords::world_to_chunk(position));
    auto& chunk = loaded_chunks_.at(hash);

    LocalCoord local_position = Coords::world_to_local(position);
    chunk->setBlockAt(local_position, block);

    return true;
}

/* non owning pointer is being returned, should be discarded right after use */
[[nodiscard]] auto World::tryGetChunk(ChunkCoord position) const -> Chunk*
{
    if (!isChunkLoaded(position)) {
        return nullptr;
    }

    std::uint64_t hash = Chunk::calculateHash(position);
    auto iterator = loaded_chunks_.find(hash);

    if (iterator == loaded_chunks_.end()) {
        return nullptr;
    }

    return iterator->second.get();
}

[[nodiscard]] auto World::getSeed() const noexcept -> int
{
    return seed_;
}

void World::setSeed(int seed) noexcept
{
    seed_ = seed;
    noise_generator_.SetSeed(seed_);
}

auto World::getLocalPlayer() noexcept -> Player*
{
    return local_player_.get();
}

auto World::getEvents() -> std::queue<WorldEvent>&
{
    return events_;
}

auto World::generateChunk(ChunkCoord position) -> Chunk&
{
    std::uint64_t hash = Chunk::calculateHash(position);

    auto [iterator, has_emplaced] = loaded_chunks_.try_emplace(hash, std::make_unique<Chunk>(position));

    for (unsigned char xpos = 0; xpos < CHUNK_SIZE_X; xpos++) {
        for (unsigned char zpos = 0; zpos < CHUNK_SIZE_Z; zpos++) {
            float value = noise_generator_.GetNoise(static_cast<float>(xpos + (position.x * CHUNK_SIZE_X)),
                                                    static_cast<float>(zpos + (position.z * CHUNK_SIZE_Z)));
            int height = static_cast<int>(((value + 1.0F) / 2.0F) * 64);

            for (unsigned char ypos = 0; ypos < CHUNK_SIZE_Y; ypos++) {
                if (ypos + (position.y * static_cast<int>(CHUNK_SIZE_Y)) < 32) {
                    iterator->second->setBlockAt(LocalCoord(xpos, ypos, zpos), Block::WATER);
                }
                if (ypos + (position.y * static_cast<int>(CHUNK_SIZE_Y)) < height - 1)
                    iterator->second->setBlockAt(LocalCoord(xpos, ypos, zpos), Block::STONE);
                else if (ypos + (position.y * static_cast<int>(CHUNK_SIZE_Y)) < height)
                    iterator->second->setBlockAt(LocalCoord(xpos, ypos, zpos), Block::GRASS);
            }
        }
    }

    events_.emplace(WorldEvent{WorldEventType::ChunkLoaded, hash, position});

    return *iterator->second;
}

void World::generateArea(ChunkCoord center, int x_radius, int y_radius, int z_radius)
{
    for (int xpos = -(x_radius / 2); xpos < (x_radius / 2); xpos++) {
        for (int zpos = -(z_radius / 2); zpos < (z_radius / 2); zpos++) {
            for (int ypos = -1; ypos < y_radius; ypos++) {
                generateChunk(center + ChunkCoord(xpos, ypos, zpos));
            }
        }
    }
}
