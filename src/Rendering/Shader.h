#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <string>

class Shader
{
public:
    Shader() = default;
    ~Shader();

    bool LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    void Bind() const;

    void SetMat4(const std::string& name, const glm::mat4& matrix) const;

    GLuint GetID() const
    {
        return m_programID;
    }

    void SetVec3(const std::string& name, const glm::vec3& value) const;

private:
    static std::string ReadFile(const std::string& path);
    static GLuint CompileShader(GLenum type, const std::string& source);

    GLuint m_programID = 0;
};