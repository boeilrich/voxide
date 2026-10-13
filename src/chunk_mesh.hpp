#pragma once
#include <array>

#include <glad/gl.h>
#include <glm/glm.hpp>

#include "block.hpp"
#include "coordinates.hpp"

class Chunk;
class World;

constexpr std::array<std::array<glm::uvec3, 4>, 6> corner_positions = {{
    {glm::uvec3(1, 1, 1), glm::uvec3(1, 0, 1), glm::uvec3(1, 0, 0), glm::uvec3(1, 1, 0)},  // X+
    {glm::uvec3(0, 1, 0), glm::uvec3(0, 0, 0), glm::uvec3(0, 0, 1), glm::uvec3(0, 1, 1)},  // X-
    {glm::uvec3(0, 1, 1), glm::uvec3(1, 1, 1), glm::uvec3(1, 1, 0), glm::uvec3(0, 1, 0)},  // Y+
    {glm::uvec3(1, 0, 1), glm::uvec3(0, 0, 1), glm::uvec3(0, 0, 0), glm::uvec3(1, 0, 0)},  // Y-
    {glm::uvec3(0, 1, 1), glm::uvec3(0, 0, 1), glm::uvec3(1, 0, 1), glm::uvec3(1, 1, 1)},  // Z+
    {glm::uvec3(1, 1, 0), glm::uvec3(1, 0, 0), glm::uvec3(0, 0, 0), glm::uvec3(0, 1, 0)},  // Z-
}};

constexpr std::array<std::array<std::array<glm::ivec3, 3>, 4>, 6> ao_lookup = {{
    // Face 0: X+ (normal +X), tangents: Y and Z
    // corners: (1,1,1), (1,0,1), (1,0,0), (1,1,0)
    {{
        {{glm::ivec3(1, 1, 0), glm::ivec3(1, 0, 1), glm::ivec3(1, 1, 1)}},      // (1,1,1): +Y, +Z
        {{glm::ivec3(1, -1, 0), glm::ivec3(1, 0, 1), glm::ivec3(1, -1, 1)}},    // (1,0,1): -Y, +Z
        {{glm::ivec3(1, -1, 0), glm::ivec3(1, 0, -1), glm::ivec3(1, -1, -1)}},  // (1,0,0): -Y, -Z
        {{glm::ivec3(1, 1, 0), glm::ivec3(1, 0, -1), glm::ivec3(1, 1, -1)}},    // (1,1,0): +Y, -Z
    }},
    // Face 1: X- (normal -X), tangents: Y and Z
    // corners: (0,1,0), (0,0,0), (0,0,1), (0,1,1)
    {{
        {{glm::ivec3(-1, 1, 0), glm::ivec3(-1, 0, -1), glm::ivec3(-1, 1, -1)}},    // (0,1,0): +Y, -Z
        {{glm::ivec3(-1, -1, 0), glm::ivec3(-1, 0, -1), glm::ivec3(-1, -1, -1)}},  // (0,0,0): -Y, -Z
        {{glm::ivec3(-1, -1, 0), glm::ivec3(-1, 0, 1), glm::ivec3(-1, -1, 1)}},    // (0,0,1): -Y, +Z
        {{glm::ivec3(-1, 1, 0), glm::ivec3(-1, 0, 1), glm::ivec3(-1, 1, 1)}},      // (0,1,1): +Y, +Z
    }},
    // Face 2: Y+ (normal +Y), tangents: X and Z
    // corners: (0,1,1), (1,1,1), (1,1,0), (0,1,0)
    {{
        {{glm::ivec3(-1, 1, 0), glm::ivec3(0, 1, 1), glm::ivec3(-1, 1, 1)}},    // (0,1,1): -X, +Z
        {{glm::ivec3(1, 1, 0), glm::ivec3(0, 1, 1), glm::ivec3(1, 1, 1)}},      // (1,1,1): +X, +Z
        {{glm::ivec3(1, 1, 0), glm::ivec3(0, 1, -1), glm::ivec3(1, 1, -1)}},    // (1,1,0): +X, -Z
        {{glm::ivec3(-1, 1, 0), glm::ivec3(0, 1, -1), glm::ivec3(-1, 1, -1)}},  // (0,1,0): -X, -Z
    }},
    // Face 3: Y- (normal -Y), tangents: X and Z
    // corners: (1,0,1), (0,0,1), (0,0,0), (1,0,0)
    {{
        {{glm::ivec3(1, -1, 0), glm::ivec3(0, -1, 1), glm::ivec3(1, -1, 1)}},      // (1,0,1): +X, +Z
        {{glm::ivec3(-1, -1, 0), glm::ivec3(0, -1, 1), glm::ivec3(-1, -1, 1)}},    // (0,0,1): -X, +Z
        {{glm::ivec3(-1, -1, 0), glm::ivec3(0, -1, -1), glm::ivec3(-1, -1, -1)}},  // (0,0,0): -X, -Z
        {{glm::ivec3(1, -1, 0), glm::ivec3(0, -1, -1), glm::ivec3(1, -1, -1)}},    // (1,0,0): +X, -Z
    }},
    // Face 4: Z+ (normal +Z), tangents: X and Y
    // corners: (0,1,1), (0,0,1), (1,0,1), (1,1,1)
    {{
        {{glm::ivec3(-1, 0, 1), glm::ivec3(0, 1, 1), glm::ivec3(-1, 1, 1)}},    // (0,1,1): -X, +Y
        {{glm::ivec3(-1, 0, 1), glm::ivec3(0, -1, 1), glm::ivec3(-1, -1, 1)}},  // (0,0,1): -X, -Y
        {{glm::ivec3(1, 0, 1), glm::ivec3(0, -1, 1), glm::ivec3(1, -1, 1)}},    // (1,0,1): +X, -Y
        {{glm::ivec3(1, 0, 1), glm::ivec3(0, 1, 1), glm::ivec3(1, 1, 1)}},      // (1,1,1): +X, +Y
    }},
    // Face 5: Z- (normal -Z), tangents: X and Y
    // corners: (1,1,0), (1,0,0), (0,0,0), (0,1,0)
    {{
        {{glm::ivec3(1, 0, -1), glm::ivec3(0, 1, -1), glm::ivec3(1, 1, -1)}},      // (1,1,0): +X, +Y
        {{glm::ivec3(1, 0, -1), glm::ivec3(0, -1, -1), glm::ivec3(1, -1, -1)}},    // (1,0,0): +X, -Y
        {{glm::ivec3(-1, 0, -1), glm::ivec3(0, -1, -1), glm::ivec3(-1, -1, -1)}},  // (0,0,0): -X, -Y
        {{glm::ivec3(-1, 0, -1), glm::ivec3(0, 1, -1), glm::ivec3(-1, 1, -1)}},    // (0,1,0): -X, +Y
    }},
}};

enum Direction : std::uint8_t {
    XPos = 0,
    XNeg = 1,
    YPos = 2,
    YNeg = 3,
    ZPos = 4,
    ZNeg = 5,
};

using VertexData = struct VertexData {
    glm::uvec3 position;
    std::uint16_t texture;
    Direction direction;
    std::uint8_t corner;
    std::uint8_t ambient_occlusion;
};

class ChunkMesh
{
  private:
    GLuint m_VAO = 0;
    GLuint m_VBO = 0;
    GLuint m_EBO = 0;

    GLsizei m_index_count = 0;

    ChunkCoord m_position{};

    [[nodiscard]] auto packVertexData(VertexData data) const noexcept -> std::uint32_t;

    auto getBlock(const Chunk& chunk, const std::array<Chunk*, 26>& neighbors, WorldCoord coord) -> Block;

    auto generateAOValues(const Chunk& chunk, const std::array<Chunk*, 26>& neighbors, Direction direction,
                          WorldCoord coord) -> std::array<std::uint8_t, 4>;

  public:
    ChunkMesh(ChunkCoord position);
    ~ChunkMesh();

    // disable copying
    ChunkMesh(const ChunkMesh&) = delete;
    auto operator=(const ChunkMesh&) -> ChunkMesh& = delete;

    // enable moving
    ChunkMesh(ChunkMesh&&) noexcept;
    auto operator=(ChunkMesh&&) noexcept -> ChunkMesh&;

    [[nodiscard]] auto getPosition() const noexcept -> ChunkCoord;

    void generate(const Chunk& chunk, const World& world);
    void render() const noexcept;
};
