#include "chunk.hpp"

#include "block.hpp"
#include "chunk_mesh.hpp"
#include "coordinates.hpp"

void Chunk::initializeBlockArray()
{
    if (!isEmpty()) {
        return;
    }

    m_blocks = std::make_unique<std::array<Block, CHUNK_SIZE>>();
    m_blocks->fill(Block::AIR);
}

auto Chunk::getBlockAt(LocalCoord position) const noexcept -> Block
{
    if (isEmpty()) {
        return Block::AIR;
    }

    return (*m_blocks)[position.to_index()];
}

void Chunk::setBlockAt(LocalCoord position, Block block, bool supress_dirty)
{
    if (isEmpty() && block == Block::AIR) {
        return;
    }

    if (isEmpty()) {
        initializeBlockArray();
    }

    const std::uint16_t index = position.to_index();
    Block current_block = (*this->m_blocks)[index];

    if (current_block != block) {
        (*m_blocks)[index] = block;

        if (block != Block::AIR && current_block == Block::AIR) {
            m_air_block_count--;
        } else if (block == Block::AIR && current_block != Block::AIR) {
            m_air_block_count++;
        }

        // delete chunk array if chunk is completly made of air
        if (m_air_block_count == CHUNK_SIZE) {
            m_blocks = nullptr;
        }
    }

    if (!supress_dirty) {
        m_dirty_flag = true;
    }
}

auto Chunk::getPosition() const noexcept -> ChunkCoord
{
    return m_position;
}

auto Chunk::calculateHash(const ChunkCoord& position) noexcept -> uint64_t
{
    auto xpos = static_cast<std::uint64_t>(static_cast<std::uint32_t>(position.x));
    auto ypos = static_cast<std::uint64_t>(static_cast<std::uint32_t>(position.y));
    auto zpos = static_cast<std::uint64_t>(static_cast<std::uint32_t>(position.z));

    std::uint64_t hash = 0;

    hash ^= xpos * 0x9E3779B185EBCA87ULL;
    hash ^= ypos * 0xC2B2AE3D27D4EB4FULL;
    hash ^= zpos * 0x165667B19E3779F9ULL;

    hash ^= (hash >> 30U);
    hash *= 0xbf58476d1ce4e5bULL;
    hash ^= (hash >> 27U);
    hash *= 0x94d049bb133111ebULL;
    hash ^= (hash >> 31U);

    return hash;
}

auto Chunk::getHash() const noexcept -> std::uint64_t
{
    return Chunk::calculateHash(m_position);
}

auto Chunk::isDirty() const noexcept -> bool
{
    return m_dirty_flag;
}

void Chunk::setDirty(bool dirty) noexcept
{
    m_dirty_flag = dirty;
}

auto Chunk::isEmpty() const noexcept -> bool
{
    return m_air_block_count == CHUNK_SIZE;
}
