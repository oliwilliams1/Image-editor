#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/vec3.hpp>

struct ImageEditData
{
	float gamma; // [0.2, 5.0]

	float exposure; // [-5, 5] working ev
	float gamutMap; // [0 or 1] switch
	float reinhard; // [0 or 1] switch

	float colTempS, colTempT; // Start temperature, target temperature [2000k, 10000k]

	glm::vec3 colBalance; // [-1, 1] cyan <-> red, magenta <-> blue, yellow <-> blue
	float keepLumaColBalance; // [0 or 1] switch

	float hue; // [-180, 180] degrees
	float saturation; // [0, 5]
	float invert; // [0 or 1]

	glm::vec3 avgColour;

	ImageEditData() : exposure(0.0f), gamutMap(0.0f), reinhard(0.0f), gamma(2.2f), 
		colTempS(6500.0f), colTempT(6500.0f), colBalance(0.0f), keepLumaColBalance(0.0f), 
		hue(0.0f), saturation(0.0f), invert(0.0f), avgColour(0.0f) {}
};

struct Image
{
	GLuint textureID;
	std::string filePath;
	int width, height, channels;
	ImageEditData editData;
};

glm::vec3 GetAvgColour(GLuint textureID, GLint maxLevel);
void SetupImGuiStyle();
Image LoadImage(const std::string& filePath);
bool ReadFile(const char* pFileName, std::string& outFile);
void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);