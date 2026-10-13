#pragma once

#include <optional>
#include <string>

#include <glad/gl.h>
#include <glm/glm.hpp>

class ShaderProgram
{
  private:
    GLuint m_program_id = 0;

    auto loadAndCompile() -> bool;

    [[nodiscard]] static auto createShader(const std::string& filepath, int shader_type) -> std::optional<GLuint>;
    static auto loadShaderFile(const std::string& path) -> std::optional<std::string>;

    std::string m_vertex_shader_path;
    std::string m_fragment_shader_path;

  public:
    ShaderProgram(std::string vertex_shader_path, std::string fragment_shader_path);
    ~ShaderProgram();

    // disable copying
    ShaderProgram(const ShaderProgram&) = delete;
    auto operator=(const ShaderProgram&) -> ShaderProgram& = delete;

    // Enable cheap moving
    ShaderProgram(ShaderProgram&&) noexcept = default;
    auto operator=(ShaderProgram&&) noexcept -> ShaderProgram& = default;

    [[nodiscard]] auto use() const -> bool;
    auto reload() -> bool;
    void setUniform(const std::string& name, glm::mat4 value) const;
};
