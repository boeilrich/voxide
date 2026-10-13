#include "world.hpp"

#include <cstdint>
#include <memory>

#include <spdlog/spdlog.h>

#include "chunk.hpp"
#include "coordinates.hpp"
#include "player.hpp"

World::World(int seed) : m_seed(seed)
{
    m_local_player = std::make_unique<Player>(true);
    initializeNoiseGenerator();
}

void World::initializeNoiseGenerator()
{
    m_noise_generator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    m_noise_generator.SetFrequency(0.02F);
    m_noise_generator.SetFractalType(FastNoiseLite::FractalType_FBm);
    m_noise_generator.SetFractalOctaves(3);
    m_noise_generator.SetFractalLacunarity(2.0F);
    m_noise_generator.SetFractalGain(0.7F);

    m_noise_generator.SetSeed(m_seed);
}

auto World::isChunkLoaded(ChunkCoord position) const noexcept -> bool
{
    std::uint64_t hash = Chunk::calculateHash(position);
    return m_loaded_chunks.contains(hash);
}

void World::tick()
{
    for (const auto& entry : m_loaded_chunks) {
        if (entry.second->isDirty()) {
            m_events.emplace(WorldEvent{WorldEventType::ChunkDirty, entry.first, entry.second->getPosition()});
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
    const auto& chunk = m_loaded_chunks.at(hash);

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
    auto& chunk = m_loaded_chunks.at(hash);

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
    auto iterator = m_loaded_chunks.find(hash);

    if (iterator == m_loaded_chunks.end()) {
        return nullptr;
    }

    return iterator->second.get();
}

[[nodiscard]] auto World::getSeed() const noexcept -> int
{
    return m_seed;
}

void World::setSeed(int seed) noexcept
{
    m_seed = seed;
    m_noise_generator.SetSeed(m_seed);
}

auto World::getLocalPlayer() noexcept -> Player*
{
    return m_local_player.get();
}

auto World::getEvents() -> std::queue<WorldEvent>&
{
    return m_events;
}

auto World::generateChunk(ChunkCoord position) -> Chunk&
{
    std::uint64_t hash = Chunk::calculateHash(position);

    auto [iterator, has_emplaced] = m_loaded_chunks.try_emplace(hash, std::make_unique<Chunk>(position));

    for (unsigned char xpos = 0; xpos < CHUNK_SIZE_X; xpos++) {
        for (unsigned char zpos = 0; zpos < CHUNK_SIZE_Z; zpos++) {
            float value = m_noise_generator.GetNoise(static_cast<float>(xpos + (position.x * CHUNK_SIZE_X)),
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

    m_events.emplace(WorldEvent{WorldEventType::ChunkLoaded, hash, position});

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
