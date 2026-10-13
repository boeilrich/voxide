#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>
#include <glm/vec3.hpp>

constexpr uint8_t CHUNK_SIZE_X = 32;
constexpr uint8_t CHUNK_SIZE_Y = 32;
constexpr uint8_t CHUNK_SIZE_Z = 32;
constexpr uint32_t CHUNK_SIZE
    = static_cast<std::uint32_t>(CHUNK_SIZE_X) * static_cast<std::uint32_t>(CHUNK_SIZE_Y) * static_cast<std::uint32_t>(CHUNK_SIZE_Z);

struct LocalCoord
{
    uint8_t x, y, z;

    [[nodiscard]] static auto from(int xpos, int ypos, int zpos) noexcept -> LocalCoord
    {
        assert(xpos >= 0 && xpos < CHUNK_SIZE_X && "LocalCoord x out of range");
        assert(ypos >= 0 && ypos < CHUNK_SIZE_Y && "LocalCoord y out of range");
        assert(zpos >= 0 && zpos < CHUNK_SIZE_Z && "LocalCoord z out of range");
        return LocalCoord { .x = static_cast<uint8_t>(xpos), .y = static_cast<uint8_t>(ypos), .z = static_cast<uint8_t>(zpos) };
    }

    [[nodiscard]] static auto from_vec(glm::vec3 pos) noexcept -> LocalCoord
    {
        assert(pos.x >= 0 && pos.x < CHUNK_SIZE_X && "LocalCoord x out of range");
        assert(pos.y >= 0 && pos.y < CHUNK_SIZE_Y && "LocalCoord y out of range");
        assert(pos.z >= 0 && pos.z < CHUNK_SIZE_Z && "LocalCoord z out of range");
        return LocalCoord { .x = static_cast<uint8_t>(pos.x), .y = static_cast<uint8_t>(pos.y), .z = static_cast<uint8_t>(pos.z) };
    }

    [[nodiscard]] static auto from_index(std::uint16_t index) noexcept -> LocalCoord
    {
        std::uint16_t stride = CHUNK_SIZE_Y * CHUNK_SIZE_Z;
        std::uint16_t remainder = index % stride;

        return LocalCoord {
            .x = static_cast<std::uint8_t>(index / stride),
            .y = static_cast<std::uint8_t>(remainder / CHUNK_SIZE_Z),
            .z = static_cast<std::uint8_t>(remainder % CHUNK_SIZE_Z),
        };
    }

    [[nodiscard]] auto to_index() const noexcept -> std::uint16_t
    {
        std::uint16_t stride = CHUNK_SIZE_Y * CHUNK_SIZE_Z;
        return static_cast<std::uint16_t>((this->x * stride) + (this->y * CHUNK_SIZE_Z) + this->z);
    }

    [[nodiscard]] auto operator==(const LocalCoord& other) const noexcept -> bool
    {
        return this->x == other.x && this->y == other.y && this->z == other.z;
    }
};

struct ChunkCoord
{
    int32_t x, y, z;

    [[nodiscard]] auto operator+(const ChunkCoord& other) const noexcept -> ChunkCoord
    {
        return ChunkCoord { .x = this->x + other.x, .y = this->y + other.y, .z = this->z + other.z };
    }

    [[nodiscard]] auto operator-(const ChunkCoord& other) const noexcept -> ChunkCoord
    {
        return ChunkCoord { .x = this->x - other.x, .y = this->y - other.y, .z = this->z - other.z };
    }

    [[nodiscard]] auto operator==(const ChunkCoord& other) const noexcept -> bool
    {
        return this->x == other.x && this->y == other.y && this->z == other.z;
    }

    [[nodiscard]] auto operator!=(const ChunkCoord& other) const noexcept -> bool
    {
        return !(*this == other);
    }
};

struct WorldCoord
{
    int32_t x, y, z;

    [[nodiscard]] auto operator==(const WorldCoord& other) const noexcept -> bool
    {
        return this->x == other.x && this->y == other.y && this->z == other.z;
    }

    [[nodiscard]] auto operator!=(const WorldCoord& other) const noexcept -> bool
    {
        return !(*this == other);
    }

    [[nodiscard]] auto operator+(const WorldCoord& other) const noexcept -> WorldCoord
    {
        return WorldCoord { .x = this->x + other.x, .y = this->y + other.y, .z = this->z + other.z };
    }

    [[nodiscard]] auto operator-(const WorldCoord& other) const noexcept -> WorldCoord
    {
        return WorldCoord { .x = this->x - other.x, .y = this->y - other.y, .z = this->z - other.z };
    }

    [[nodiscard]] auto from_vec3(const glm::vec3& vec) noexcept -> WorldCoord
    {
        return WorldCoord { .x = static_cast<int32_t>(std::floor(vec.x)),
                            .y = static_cast<int32_t>(std::floor(vec.y)),
                            .z = static_cast<int32_t>(std::floor(vec.z)) };
    }
};

namespace Coords
{

[[nodiscard]] inline auto floor_div(int32_t first, int32_t second) noexcept -> int32_t
{
    return (first / second) - (first % second != 0 && (first ^ second) < 0);
}

[[nodiscard]] inline auto floor_mod(int32_t first, int32_t second) noexcept -> int32_t
{
    return first - (floor_div(first, second) * second);
}

[[nodiscard]] inline auto world_to_chunk(const WorldCoord& position) noexcept -> ChunkCoord
{
    return ChunkCoord { .x = floor_div(position.x, CHUNK_SIZE_X),
                        .y = floor_div(position.y, CHUNK_SIZE_Y),
                        .z = floor_div(position.z, CHUNK_SIZE_Z) };
}

[[nodiscard]] inline auto world_to_local(const WorldCoord& position) noexcept -> LocalCoord
{
    return LocalCoord::from(floor_mod(position.x, CHUNK_SIZE_X), floor_mod(position.y, CHUNK_SIZE_Y), floor_mod(position.z, CHUNK_SIZE_Z));
}

[[nodiscard]] inline auto to_world(const ChunkCoord& chunk_position, const LocalCoord& local_position) noexcept -> WorldCoord
{
    return WorldCoord { .x = (chunk_position.x * CHUNK_SIZE_X) + static_cast<int32_t>(local_position.x),
                        .y = (chunk_position.y * CHUNK_SIZE_Y) + static_cast<int32_t>(local_position.y),
                        .z = (chunk_position.z * CHUNK_SIZE_Z) + static_cast<int32_t>(local_position.z) };
}

[[nodiscard]] inline auto chunk_origin(const ChunkCoord& position) noexcept -> WorldCoord
{
    return WorldCoord { .x = position.x * CHUNK_SIZE_X, .y = position.y * CHUNK_SIZE_Y, .z = position.z * CHUNK_SIZE_Z };
}

} // namespace Coords
