#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/vec3.hpp>

struct Image
{
	GLuint textureID;
	std::string filePath;
	int width, height, channels;
	glm::vec3 avgColour;
};

glm::vec3 GetAvgColour(GLuint textureID, GLint maxLevel);
void SetupImGuiStyle();
Image LoadImage(const std::string& filePath);
bool ReadFile(const char* pFileName, std::string& outFile);
void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);