#pragma once

#include <cstdint>
#include <memory>

#include <glm/glm.hpp>

#include "block.hpp"
#include "chunk_mesh.hpp"
#include "coordinates.hpp"

class Chunk
{
  private:
    ChunkCoord position_;
    bool is_dirty_ = true;

    std::unique_ptr<std::array<Block, CHUNK_SIZE>> blocks_ = nullptr;
    std::uint16_t air_block_count_ = CHUNK_SIZE;

    void initializeBlockArray();

  public:
    Chunk(ChunkCoord position) : position_(position) {};
    ~Chunk() = default;

    // disable copying
    Chunk(const Chunk&) = delete;
    auto operator=(const Chunk&) -> Chunk& = delete;

    // enable moving
    Chunk(Chunk&&) noexcept = default;
    auto operator=(Chunk&&) noexcept -> Chunk& = default;

    [[nodiscard]] auto getBlockAt(LocalCoord position) const noexcept -> Block;
    void setBlockAt(LocalCoord position, Block block, bool supress_dirty = false);

    [[nodiscard]] auto getPosition() const noexcept -> ChunkCoord;

    static auto calculateHash(const ChunkCoord& position) noexcept -> std::uint64_t;
    [[nodiscard]] auto getHash() const noexcept -> std::uint64_t;

    [[nodiscard]] auto isDirty() const noexcept -> bool;
    void setDirty(bool dirty) noexcept;

    [[nodiscard]] auto isEmpty() const noexcept -> bool;
};
