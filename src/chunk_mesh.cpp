#include "chunk_mesh.hpp"

#include <array>
#include <cstdint>

#include <glad/gl.h>
#include <glm/fwd.hpp>
#include <spdlog/spdlog.h>

#include "block.hpp"
#include "chunk.hpp"
#include "coordinates.hpp"
#include "world.hpp"

ChunkMesh::ChunkMesh(ChunkCoord position) : position_(position)
{
    glGenBuffers(1, &VBO_);
    glGenBuffers(1, &EBO_);
    glGenVertexArrays(1, &VAO_);

    if (VBO_ == 0 || EBO_ == 0) {
        GLenum error = glGetError();
        spdlog::error("glGenBuffers returned an Error: {} ", error);
        throw std::runtime_error("glGenBuffers returned an Error");
    }
    if (VAO_ == 0) {
        GLenum error = glGetError();
        spdlog::error("glGenVertexArrays returned an Error: {}", error);
        throw std::runtime_error("glGenVertexArrays returned an Error");
    }
}

ChunkMesh::~ChunkMesh()
{
    glDeleteVertexArrays(1, &VAO_);
    glDeleteBuffers(1, &VBO_);
    glDeleteBuffers(1, &EBO_);
}

ChunkMesh::ChunkMesh(ChunkMesh&& other) noexcept
    : VAO_(other.VAO_),
      VBO_(other.VBO_),
      EBO_(other.EBO_),
      index_count_(other.index_count_)
{
    other.VAO_ = 0;
    other.VBO_ = 0;
    other.EBO_ = 0;
    other.index_count_ = 0;
}

auto ChunkMesh::operator=(ChunkMesh&& other) noexcept -> ChunkMesh&
{
    if (this == &other) {
        return *this;
    }

    if (VAO_)
        glDeleteVertexArrays(1, &VAO_);
    if (VBO_)
        glDeleteBuffers(1, &VBO_);
    if (EBO_)
        glDeleteBuffers(1, &EBO_);

    VAO_ = other.VAO_;
    VBO_ = other.VBO_;
    EBO_ = other.EBO_;
    index_count_ = other.index_count_;

    other.VAO_ = 0;
    other.VBO_ = 0;
    other.EBO_ = 0;
    other.index_count_ = 0;

    return *this;
}

auto ChunkMesh::packVertexData(VertexData data) const noexcept -> std::uint32_t
{
    std::uint32_t packed = 0;

    packed |= static_cast<std::uint32_t>(data.position.x & 0x3FU) << 26U;
    packed |= static_cast<std::uint32_t>(data.position.y & 0x3FU) << 20U;
    packed |= static_cast<std::uint32_t>(data.position.z & 0x3FU) << 14U;

    packed |= static_cast<std::uint32_t>(data.texture & 0x7FU) << 7U;

    packed |= static_cast<std::uint32_t>(data.direction & 0x7U) << 4U;
    packed |= static_cast<std::uint32_t>(data.corner & 0x3U) << 2U;
    packed |= static_cast<std::uint32_t>(data.ambient_occlusion & 0x3U) << 0U;
    return packed;
}

auto ChunkMesh::getBlock(const Chunk& chunk, const std::array<Chunk*, 26>& neighbors, WorldCoord coord) -> Block
{
    auto chunk_coord = Coords::world_to_chunk(coord);
    if (chunk_coord == chunk.getPosition()) {
        return chunk.getBlockAt(Coords::world_to_local(coord));
    }

    for (Chunk* neighbor : neighbors) {
        if (neighbor && chunk_coord == neighbor->getPosition()) {
            return neighbor->getBlockAt(Coords::world_to_local(coord));
        }
    }

    return Block::AIR;
}
auto ChunkMesh::generateAOValues(const Chunk& chunk, const std::array<Chunk*, 26>& neighbors, Direction direction,
                                 WorldCoord coord) -> std::array<std::uint8_t, 4>
{
    std::array<std::uint8_t, 4> ao_out{};

    for (unsigned long corner = 0; corner < 4; corner++) {
        const auto& samples = ao_lookup[direction][corner];

        auto sample_offset = [&](const glm::ivec3& offset) -> Block {
            return this->getBlock(chunk, neighbors,
                                  WorldCoord{.x = coord.x + offset.x,
                                             .y = coord.y + offset.y,
                                             .z = coord.z + offset.z});
        };

        const Block side1 = sample_offset(samples[0]);
        const Block side2 = sample_offset(samples[1]);
        const Block diagonal = sample_offset(samples[2]);

        const auto is_solid = [](Block block) { return block != Block::AIR; };

        ao_out[corner] = (is_solid(side1) && is_solid(side2))
                             ? 3
                             : static_cast<uint8_t>(is_solid(side1) + is_solid(side2) + is_solid(diagonal));
    }
    return ao_out;
}

[[nodiscard]] auto ChunkMesh::getPosition() const noexcept -> ChunkCoord
{
    return this->position_;
}

void ChunkMesh::generate(const Chunk& chunk, const World& world)
{
    std::vector<std::uint32_t> packed_vertex_data;
    std::vector<GLuint> indicies;

    packed_vertex_data.reserve(1024);  // 6 faces, 4 verts each
    indicies.reserve(1536);

    auto push_face = [&](glm::uvec3 pos, Direction direction, Block block,
                         std::array<std::uint8_t, 4> ambient_occlusion) -> void {
        GLuint base = static_cast<GLuint>(packed_vertex_data.size());
        indicies.insert(indicies.end(), {base + 0, base + 1, base + 2, base + 0, base + 2, base + 3});
        packed_vertex_data.push_back(this->packVertexData({.position = pos + corner_positions[direction][0],
                                                           .texture = static_cast<std::uint8_t>(block),
                                                           .direction = direction,
                                                           .corner = 0,
                                                           .ambient_occlusion = ambient_occlusion[0]}));
        packed_vertex_data.push_back(this->packVertexData({.position = pos + corner_positions[direction][1],
                                                           .texture = static_cast<std::uint8_t>(block),
                                                           .direction = direction,
                                                           .corner = 1,
                                                           .ambient_occlusion = ambient_occlusion[1]}));
        packed_vertex_data.push_back(this->packVertexData({.position = pos + corner_positions[direction][2],
                                                           .texture = static_cast<std::uint8_t>(block),
                                                           .direction = direction,
                                                           .corner = 2,
                                                           .ambient_occlusion = ambient_occlusion[2]}));
        packed_vertex_data.push_back(this->packVertexData({.position = pos + corner_positions[direction][3],
                                                           .texture = static_cast<std::uint8_t>(block),
                                                           .direction = direction,
                                                           .corner = 3,
                                                           .ambient_occlusion = ambient_occlusion[3]}));
    };

    std::array<Chunk*, 26> neighbors{};

    unsigned long index = 0;
    for (int xpos = -1; xpos <= 1; xpos++) {
        for (int ypos = -1; ypos <= 1; ypos++) {
            for (int zpos = -1; zpos <= 1; zpos++) {
                if (xpos == 0 && ypos == 0 && zpos == 0) {
                    continue;
                }
                neighbors[index++] =
                    world.tryGetChunk(chunk.getPosition() + ChunkCoord{.x = xpos, .y = ypos, .z = zpos});
            }
        }
    }

    const WorldCoord chunk_origin = Coords::chunk_origin(chunk.getPosition());

    for (std::uint8_t xpos = 0; xpos < CHUNK_SIZE_X; xpos++) {
        for (std::uint8_t ypos = 0; ypos < CHUNK_SIZE_Y; ypos++) {
            for (std::uint8_t zpos = 0; zpos < CHUNK_SIZE_Z; zpos++) {
                const Block current_block = chunk.getBlockAt(LocalCoord::from(xpos, ypos, zpos));
                if (current_block == Block::AIR)
                    continue;

                const WorldCoord coord = {.x = chunk_origin.x + xpos,
                                          .y = chunk_origin.y + ypos,
                                          .z = chunk_origin.z + zpos};

                auto neighbor_block = [&](int delta_x, int delta_y, int delta_z) -> Block {
                    const int neighbor_x = xpos + delta_x;
                    const int neighbor_y = ypos + delta_y;
                    const int neighbor_z = zpos + delta_z;
                    if (neighbor_x >= 0 && neighbor_x < CHUNK_SIZE_X && neighbor_y >= 0 && neighbor_y < CHUNK_SIZE_Y &&
                        neighbor_z >= 0 && neighbor_z < CHUNK_SIZE_Z) {
                        return chunk.getBlockAt(LocalCoord::from(neighbor_x, neighbor_y, neighbor_z));
                    }
                    return this->getBlock(chunk, neighbors,
                                          WorldCoord{.x = coord.x + delta_x,
                                                     .y = coord.y + delta_y,
                                                     .z = coord.z + delta_z});
                };

                if (neighbor_block(1, 0, 0) == Block::AIR)
                    push_face({xpos, ypos, zpos}, Direction::XPos, current_block,
                              generateAOValues(chunk, neighbors, Direction::XPos, coord));
                if (neighbor_block(-1, 0, 0) == Block::AIR)
                    push_face({xpos, ypos, zpos}, Direction::XNeg, current_block,
                              generateAOValues(chunk, neighbors, Direction::XNeg, coord));
                if (neighbor_block(0, 1, 0) == Block::AIR)
                    push_face({xpos, ypos, zpos}, Direction::YPos, current_block,
                              generateAOValues(chunk, neighbors, Direction::YPos, coord));
                if (neighbor_block(0, -1, 0) == Block::AIR)
                    push_face({xpos, ypos, zpos}, Direction::YNeg, current_block,
                              generateAOValues(chunk, neighbors, Direction::YNeg, coord));
                if (neighbor_block(0, 0, 1) == Block::AIR)
                    push_face({xpos, ypos, zpos}, Direction::ZPos, current_block,
                              generateAOValues(chunk, neighbors, Direction::ZPos, coord));
                if (neighbor_block(0, 0, -1) == Block::AIR)
                    push_face({xpos, ypos, zpos}, Direction::ZNeg, current_block,
                              generateAOValues(chunk, neighbors, Direction::ZNeg, coord));
            }
        }
    }

    glBindVertexArray(this->VAO_);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO_);

    auto verticies_size = static_cast<GLsizei>(packed_vertex_data.size() * sizeof(std::uint32_t));
    auto indicies_size = static_cast<GLsizei>(indicies.size() * sizeof(std::uint32_t));

    glBufferData(GL_ARRAY_BUFFER, verticies_size, packed_vertex_data.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies_size, indicies.data(), GL_STATIC_DRAW);

    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(std::uint32_t), nullptr);
    glEnableVertexAttribArray(0);

    this->index_count_ = static_cast<GLsizei>(indicies.size());
}

void ChunkMesh::render() const noexcept
{
    glBindVertexArray(this->VAO_);
    glDrawElements(GL_TRIANGLES, this->index_count_, GL_UNSIGNED_INT, nullptr);
}
