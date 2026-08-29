#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>


Shader::~Shader()
{
    if (m_programID != 0)
    {
        glDeleteProgram(m_programID);
    }
}


std::string Shader::ReadFile(const std::string& path)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        std::cerr << "Failed to open shader file: " << path << '\n';
        return {};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}


GLuint Shader::CompileShader(
    GLenum type,
    const std::string& source)
{
    GLuint shader = glCreateShader(type);

    const char* sourcePtr = source.c_str();

    glShaderSource
    (
        shader,
        1,
        &sourcePtr,
        nullptr
    );

    glCompileShader(shader);

    GLint success = GL_FALSE;

    glGetShaderiv
    (
        shader,
        GL_COMPILE_STATUS,
        &success
    );

    if (success != GL_TRUE)
    {
        GLint logLength = 0;

        glGetShaderiv
        (
            shader,
            GL_INFO_LOG_LENGTH,
            &logLength
        );

        std::vector<char> log(static_cast<size_t>(logLength));

        glGetShaderInfoLog
        (
            shader,
            logLength,
            nullptr,
            log.data()
        );

        std::cerr << "Shader compilation failed:\n" << log.data() << '\n';

        glDeleteShader(shader);

        return 0;
    }

    return shader;
}


bool Shader::LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath)
{
    const std::string vertexSource = ReadFile(vertexPath);
    const std::string fragmentSource = ReadFile(fragmentPath);

    if (vertexSource.empty() || fragmentSource.empty()) return false;

    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

    if (vertexShader == 0 || fragmentShader == 0)
    {
        if (vertexShader != 0)
        {
            glDeleteShader(vertexShader);
        }

        if (fragmentShader != 0)
        {
            glDeleteShader(fragmentShader);
        }

        return false;
    }

    m_programID = glCreateProgram();

    glAttachShader
    (
        m_programID,
        vertexShader
    );

    glAttachShader
    (
        m_programID,
        fragmentShader
    );

    glLinkProgram(m_programID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint success = GL_FALSE;

    glGetProgramiv
    (
        m_programID,
        GL_LINK_STATUS,
        &success
    );

    if (success != GL_TRUE)
    {
        GLint logLength = 0;

        glGetProgramiv
        (
            m_programID,
            GL_INFO_LOG_LENGTH,
            &logLength
        );

        std::vector<char> log(static_cast<size_t>(logLength));

        glGetProgramInfoLog
        (
            m_programID,
            logLength,
            nullptr,
            log.data()
        );

        std::cerr << "Shader linking failed:\n" << log.data() << '\n';

        glDeleteProgram(m_programID);
        m_programID = 0;

        return false;
    }

    return true;
}


void Shader::Bind() const
{
    glUseProgram(m_programID);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& matrix) const
{
    const GLint location = glGetUniformLocation(m_programID, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const
{
    const GLint location = glGetUniformLocation( m_programID, name.c_str());
    glUniform3fv(location, 1, glm::value_ptr(value));
}