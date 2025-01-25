#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <GL/glew.h>

struct Image
{
	GLuint textureID;
	std::string filePath;
	int width, height, channels;
};

void SetupImGuiStyle();
Image LoadImage(const std::string& filePath);
bool ReadFile(const char* pFileName, std::string& outFile);
void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);