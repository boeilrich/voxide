#pragma once
#include "shader_program.hpp"
#include "world.hpp"

class WorldRenderer
{
  private:
    World* world_;

    GLuint texture_atlas_handle_ = 0;

    std::unique_ptr<ShaderProgram> program_;
    std::unordered_map<std::uint64_t, std::unique_ptr<ChunkMesh>> meshes_;

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
