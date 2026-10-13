#include "world_renderer.hpp"

#include <memory>

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <stb/stb_image.h>

#include "world.hpp"

WorldRenderer::WorldRenderer(World* world) : m_world(world)
{
    m_program = std::make_unique<ShaderProgram>("./assets/shader/vertex.glsl", "./assets/shader/fragment.glsl");

    loadTextureAtlas();
}

WorldRenderer::~WorldRenderer()
{
    glDeleteTextures(1, &m_texture_atlas_handle);
}

auto WorldRenderer::loadTextureAtlas() -> bool
{
    int atlas_width = 0;
    int atlas_height = 0;
    unsigned char* atlas_data = stbi_load("./assets/texture_atlas.png", &atlas_width, &atlas_height, nullptr, 0);

    if (atlas_data == nullptr) {
        spdlog::warn("Failed to load texture atlas file");
        return false;
    }

    // generate texture if it does not exist
    if (m_texture_atlas_handle == 0) {
        glGenTextures(1, &m_texture_atlas_handle);
    }

    glBindTexture(GL_TEXTURE_2D, m_texture_atlas_handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, atlas_width, atlas_height, 0, GL_RGB, GL_UNSIGNED_BYTE, atlas_data);

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(atlas_data);
    return true;
}

void WorldRenderer::processEvents()
{
    auto& events = m_world->getEvents();

    while (!events.empty()) {
        auto event = events.front();
        events.pop();

        switch (event.type) {
            case WorldEventType::ChunkLoaded: {
                m_meshes.emplace(event.hash, std::make_unique<ChunkMesh>(event.position));
                break;
            }
            case WorldEventType::ChunkUnloaded: {
                break;
            }
            case WorldEventType::ChunkDirty: {
                auto& mesh = m_meshes.at(event.hash);
                mesh->generate(*m_world->tryGetChunk(event.position), *m_world);
                break;
            }
            default:
                break;
        }
    }
}

void WorldRenderer::render()
{
    processEvents();

    if (!m_program->use()) {
        spdlog::error("Failed to use Shader Program");
    }

    Camera* camera = m_world->getLocalPlayer()->getCamera();

    m_program->setUniform("view", camera->getViewMatrix());
    m_program->setUniform("projection", camera->getProjectionMatrix());

    for (auto& entry : m_meshes) {
        ChunkCoord position = entry.second->getPosition();
        auto model = glm::mat4(1.0F);

        model = glm::translate(model, {position.x * static_cast<int>(CHUNK_SIZE_X),
                                       position.y * static_cast<int>(CHUNK_SIZE_Y),
                                       position.z * static_cast<int>(CHUNK_SIZE_Z)});

        m_program->setUniform("model", model);
        entry.second->render();
    }
}

auto WorldRenderer::reloadTextureAtlas() -> bool
{
    return loadTextureAtlas();
}
