#pragma once

#include <optional>
#include <string>

#include <glad/gl.h>
#include <glm/glm.hpp>

class ShaderProgram
{
  private:
    GLuint program_handle_ = 0;

    auto loadAndCompile() -> bool;

    [[nodiscard]] static auto createShader(const std::string& filepath, GLenum shader_type) -> std::optional<GLuint>;
    static auto readFile(const std::string& path) -> std::optional<std::string>;

    std::string vertex_shader_path_;
    std::string fragment_shader_path_;

    GLint stored_program_handle_ = 0;

    void storeProgram() noexcept;
    void restoreProgram() const noexcept;

  public:
    ShaderProgram(std::string vertex_shader_path, std::string fragment_shader_path);
    ~ShaderProgram();

    // disable copying
    ShaderProgram(const ShaderProgram&) = delete;
    auto operator=(const ShaderProgram&) -> ShaderProgram& = delete;

    // Enable cheap moving
    ShaderProgram(ShaderProgram&&) noexcept;
    auto operator=(ShaderProgram&&) noexcept -> ShaderProgram&;

    [[nodiscard]] auto use() const -> bool;
    auto reload() -> bool;
    void setUniform(const std::string& name, const glm::mat4& value);
    void setUniform(const std::string& name, const glm::vec3& value);
};
