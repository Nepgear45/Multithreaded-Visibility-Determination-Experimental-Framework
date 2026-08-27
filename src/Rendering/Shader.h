#pragma once

#include <glad/gl.h>

#include <string>

class Shader
{
public:
    Shader() = default;
    ~Shader();

    bool LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    void Bind() const;

    GLuint GetID() const
    {
        return m_programID;
    }

private:
    static std::string ReadFile(const std::string& path);
    static GLuint CompileShader(GLenum type, const std::string& source);

private:
    GLuint m_programID = 0;
};