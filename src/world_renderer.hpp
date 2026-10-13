#pragma once
#include "shader_program.hpp"
#include "world.hpp"

class WorldRenderer
{
  private:
    World* m_world;

    GLuint m_texture_atlas_handle = 0;

    std::unique_ptr<ShaderProgram> m_program;
    std::unordered_map<std::uint64_t, std::unique_ptr<ChunkMesh>> m_meshes;

    auto loadTextureAtlas() -> bool;
    void processEvents();

  public:
    WorldRenderer(World* world);
    ~WorldRenderer();

    // disable copying
    WorldRenderer(const WorldRenderer&) = delete;
    auto operator=(const WorldRenderer&) -> WorldRenderer& = delete;

    // disable moving
    WorldRenderer(const WorldRenderer&&) = delete;
    auto operator=(const WorldRenderer&&) -> WorldRenderer& = delete;

    void render();
    auto reloadTextureAtlas() -> bool;
};
