#include "shader_program.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

ShaderProgram::ShaderProgram(std::string vertex_shader_path, std::string fragment_shader_path)
    : vertex_shader_path_(std::move(vertex_shader_path)),
      fragment_shader_path_(std::move(fragment_shader_path))
{
    spdlog::info("creating ShaderProgram: vs={}, fs={}", vertex_shader_path_, fragment_shader_path_);

    if (!this->loadAndCompile()) {
        spdlog::error("Failed to load and compile ShaderProgram");
        throw std::runtime_error("Failed to load and compile ShaderProgram");
    }
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
{
    program_handle_ = std::exchange(other.program_handle_, 0);
    vertex_shader_path_ = std::move(other.vertex_shader_path_);
    fragment_shader_path_ = std::move(other.fragment_shader_path_);
}

auto ShaderProgram::operator=(ShaderProgram&& other) noexcept -> ShaderProgram&
{
    if (this != &other) {
        if (program_handle_ != 0) {
            glDeleteProgram(program_handle_);
        }
        program_handle_ = std::exchange(other.program_handle_, 0);
        vertex_shader_path_ = std::move(other.vertex_shader_path_);
        fragment_shader_path_ = std::move(other.fragment_shader_path_);
    }
    return *this;
}

auto ShaderProgram::loadAndCompile() -> bool
{
    std::optional<GLuint> vertex_shader = ShaderProgram::createShader(vertex_shader_path_, GL_VERTEX_SHADER);
    if (!vertex_shader.has_value()) {
        return false;
    }

    std::optional<GLuint> fragment_shader = ShaderProgram::createShader(fragment_shader_path_, GL_FRAGMENT_SHADER);
    if (!fragment_shader.has_value()) {
        glDeleteShader(vertex_shader.value());
        return false;
    }

    GLuint program_handle = glCreateProgram();
    if (program_handle == 0) {
        glDeleteShader(vertex_shader.value());
        glDeleteShader(fragment_shader.value());
        spdlog::error("glCreateProgram() returned 0");
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

        spdlog::error("Failed to link shader program (vs={}, fs={}):\n{}", vertex_shader_path_, fragment_shader_path_,
                      log);
        glDeleteShader(vertex_shader.value());
        glDeleteShader(fragment_shader.value());

        glDeleteProgram(program_handle);
        return false;
    }
    spdlog::debug("Successfully linked program: vs={}, fs={}", vertex_shader_path_, fragment_shader_path_);

    glDeleteShader(vertex_shader.value());
    glDeleteShader(fragment_shader.value());

    program_handle_ = program_handle;

    return true;
}

auto ShaderProgram::createShader(const std::string& filepath, GLenum shader_type) -> std::optional<GLuint>
{
    GLuint shader_handle = glCreateShader(shader_type);
    if (shader_handle == 0) {
        spdlog::error("glCreateShader({}) returned 0", shader_type);
        return std::nullopt;
    }

    spdlog::debug("glCreateShader({}) -> handle={}", shader_type, shader_handle);

    std::optional<std::string> shader_code = ShaderProgram::readFile(filepath);

    if (!shader_code.has_value()) {
        spdlog::error("Cannot load shader file: {}", filepath);
        glDeleteShader(shader_handle);
        return std::nullopt;
    }

    const char* c_code = shader_code->c_str();
    glShaderSource(shader_handle, 1, &c_code, nullptr);
    glCompileShader(shader_handle);

    // checking compile status of shader
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

auto ShaderProgram::readFile(const std::string& filepath) -> std::optional<std::string>
{
    std::ifstream file(filepath);

    if (!file.is_open()) {
        spdlog::error("Cannot open file: {}", filepath);
        return std::nullopt;
    }

    std::ostringstream stream;
    stream << file.rdbuf();
    file.close();

    spdlog::debug("Loaded file: {}", filepath);
    return stream.str();
}

void ShaderProgram::storeProgram() noexcept
{
    glGetIntegerv(GL_CURRENT_PROGRAM, &stored_program_handle_);
}

void ShaderProgram::restoreProgram() const noexcept
{
    glUseProgram(static_cast<GLuint>(stored_program_handle_));
}

ShaderProgram::~ShaderProgram()
{
    spdlog::debug("Destroying ShaderProgram {}", program_handle_);
    if (program_handle_ != 0) {
        glDeleteProgram(program_handle_);
        program_handle_ = 0;
    }
}

auto ShaderProgram::use() const -> bool
{
    if (program_handle_ == 0) {
        return false;
    }

    glUseProgram(program_handle_);

    return true;
}

auto ShaderProgram::reload() -> bool
{
    spdlog::info("Hot-reloading ShaderProgram {}", program_handle_);

    assert(program_handle_ != 0);
    GLuint old_handle = program_handle_;

    if (!this->loadAndCompile()) {
        spdlog::error("ShaderProgram reload failed, keeping old program");
        return false;
    }

    spdlog::info("ShaderProgram reloaded successfully (new handle={})", program_handle_);

    glDeleteProgram(old_handle);
    return true;
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat4& value)
{
    this->storeProgram();

    if (!this->use()) {
        spdlog::warn("failed to use shader program while setting uniform");
        this->restoreProgram();
        return;
    }

    GLint location = glGetUniformLocation(program_handle_, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));

    this->restoreProgram();
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec3& value)
{
    this->storeProgram();

    if (!this->use()) {
        spdlog::warn("failed to use shader program while setting uniform");
        this->restoreProgram();
        return;
    }

    GLint location = glGetUniformLocation(program_handle_, name.c_str());
    glUniform3fv(location, 1, glm::value_ptr(value));

    this->restoreProgram();
}
