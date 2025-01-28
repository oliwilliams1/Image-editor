#include "Shader.h"

Shader::Shader(const char* vsSource, const char* fsSource) 
{
    this->vsSource = vsSource;
    this->fsSource = fsSource;

    LoadBasicShaders();
}

void Shader::LoadBasicShaders() 
{
    // Simmilar to OGLDEVs implementation
    shaderProgram = glCreateProgram();
    if (shaderProgram == 0) 
    {
        std::cerr << "Error creating shader program!" << std::endl;
        exit(1);
    }

    std::string vs, fs;

    if (!ReadFile(vsSource.c_str(), vs)) exit(1);
    AddShader(shaderProgram, vs.c_str(), GL_VERTEX_SHADER);

    if (!ReadFile(fsSource.c_str(), fs)) exit(1);
    AddShader(shaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

    GLint success = 0;
    GLchar errorLog[1024] = { 0 };

    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (success == 0) 
    {
        glGetProgramInfoLog(shaderProgram, sizeof(errorLog), nullptr, errorLog);
		std::cerr << "Error linking shader program: " << errorLog << std::endl;
        exit(1);
    }

    glUseProgram(shaderProgram);
}

Shader::~Shader() 
{
    glDeleteProgram(shaderProgram);
}

void Shader::Reload() 
{
    LoadBasicShaders();
}

void Shader::use() const 
{
    glUseProgram(shaderProgram);
}