#include "shader_program.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

ShaderProgram::ShaderProgram(std::string vertex_shader_path, std::string fragment_shader_path)
    : m_vertex_shader_path(std::move(vertex_shader_path)),
      m_fragment_shader_path(std::move(fragment_shader_path))
{
    spdlog::trace("Creating ShaderProgram: vs={}, fs={}", m_vertex_shader_path, m_fragment_shader_path);

    if (!loadAndCompile()) {
        spdlog::error("Failed to load and compile ShaderProgram");
        throw std::runtime_error("Failed to load and compile ShaderProgram");
    }
}

auto ShaderProgram::loadAndCompile() -> bool
{
    GLuint program_handle = glCreateProgram();
    if (program_handle == 0) {
        spdlog::error("glCreateProgram() returned 0");
        return false;
    }

    std::optional<GLuint> vertex_shader = ShaderProgram::createShader(m_vertex_shader_path, GL_VERTEX_SHADER);
    if (!vertex_shader.has_value()) {
        return false;
    }

    std::optional<GLuint> fragment_shader = ShaderProgram::createShader(m_fragment_shader_path, GL_FRAGMENT_SHADER);
    if (!fragment_shader.has_value()) {
        glDeleteShader(vertex_shader.value());
        return false;
    }

    glAttachShader(program_handle, vertex_shader.value());
    glAttachShader(program_handle, fragment_shader.value());

    glLinkProgram(program_handle);

    GLint success = GL_FALSE;
    glGetProgramiv(program_handle, GL_LINK_STATUS, &success);

    if (success == GL_FALSE) {
        GLint log_length = 0;
        glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &log_length);

        std::string log(static_cast<unsigned long>(log_length), '\0');
        glGetProgramInfoLog(program_handle, log_length, nullptr, log.data());

        spdlog::error("Failed to link shader program (vs={}, fs={}):\n{}", m_vertex_shader_path, m_fragment_shader_path,
                      log);
        return false;
    }
    spdlog::debug("Successfully linked program: vs={}, fs={}", m_vertex_shader_path, m_fragment_shader_path);

    m_program_id = program_handle;

    glDeleteShader(vertex_shader.value());
    glDeleteShader(fragment_shader.value());

    return true;
}

auto ShaderProgram::createShader(const std::string& filepath, int shader_type) -> std::optional<GLuint>
{
    GLuint shader_handle = glCreateShader(shader_type);
    if (shader_handle == 0) {
        spdlog::error("glCreateShader({}) returned 0", shader_type);
        return std::nullopt;
    }
    spdlog::trace("glCreateShader({}) -> handle={}", shader_type, shader_handle);

    std::optional<std::string> shader_code = ShaderProgram::loadShaderFile(filepath);

    if (!shader_code.has_value()) {
        spdlog::error("Cannot load shader file: {}", filepath);
        glDeleteShader(shader_handle);
        return std::nullopt;
    }

    const char* c_code = shader_code->c_str();
    glShaderSource(shader_handle, 1, &c_code, nullptr);
    glCompileShader(shader_handle);

    GLint success = GL_FALSE;
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE) {
        GLint log_length = 0;
        glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &log_length);
        std::string log(static_cast<unsigned long>(log_length), '\0');
        glGetShaderInfoLog(shader_handle, log_length, nullptr, log.data());

        spdlog::error("Failed to compile shader ({}):\n{}", filepath, log);

        glDeleteShader(shader_handle);
        return std::nullopt;
    }
    spdlog::debug("Successfully compiled shader: {}", filepath);

    return shader_handle;
}

auto ShaderProgram::loadShaderFile(const std::string& filepath) -> std::optional<std::string>
{
    std::ifstream file(filepath);

    if (!file.is_open()) {
        spdlog::error("Cannot open shader file: {}", filepath);
        return std::nullopt;
    }

    std::ostringstream stream;
    stream << file.rdbuf();
    file.close();

    spdlog::trace("Loaded shader source: {}", filepath);
    return stream.str();
}

ShaderProgram::~ShaderProgram()
{
    spdlog::trace("Destroying ShaderProgram {}", m_program_id);
    if (m_program_id != 0) {
        glDeleteProgram(m_program_id);
        m_program_id = 0;
    }
}

[[nodiscard]] auto ShaderProgram::use() const -> bool
{
    if (m_program_id == 0) {
        return false;
    }

    glUseProgram(m_program_id);

    return true;
}

auto ShaderProgram::reload() -> bool
{
    spdlog::info("Hot-reloading ShaderProgram {}", m_program_id);

    assert(m_program_id != 0);
    GLuint old_handle = m_program_id;

    if (!loadAndCompile()) {
        spdlog::error("ShaderProgram reload failed, keeping old program");
        return false;
    }

    spdlog::info("ShaderProgram reloaded successfully (new handle={})", m_program_id);

    glDeleteProgram(old_handle);
    return true;
}

void ShaderProgram::setUniform(const std::string& name, glm::mat4 value) const
{
    GLint location = glGetUniformLocation(m_program_id, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
