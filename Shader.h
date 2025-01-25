#pragma once

#include "utils.h"

class Shader 
{
public:
    GLuint shaderProgram;
    Shader(const char* vsSource, const char* fsSource);
    ~Shader();
    void LoadBasicShaders();
    virtual void Reload();
    void use() const;
    std::string vsSource, fsSource;
};