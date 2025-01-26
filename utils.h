#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include <GL/glew.h>
#include <glm/vec3.hpp>

struct ImageEditData
{
	glm::vec3 avgColour;
	float p1 = 1.0f;
	glm::vec3 AWB_ScalingFactors;
	float p2 = 1.0f;

	float gamma; // [0.2, 5.0]

	float exposure; // [-5, 5] working ev
	float gamutMap; // [0 or 1] switch
	float reinhard; // [0 or 1] switch

	float colTempS, colTempT; // Start temperature, target temperature [2000k, 10000k]
	
	float hue; // [-180, 180] degrees
	float saturation; // [0, 5]
	float invert; // [0 or 1] switch

	float u_ApplyAwb; // [0 or 1] switch

	ImageEditData() : exposure(0.0f), gamutMap(0.0f), reinhard(0.0f), gamma(2.2f), 
		colTempS(6500.0f), colTempT(6500.0f), 
		hue(0.0f), saturation(1.0f), invert(0.0f), avgColour(0.0f), 
		u_ApplyAwb(0.0f), AWB_ScalingFactors(1.0f) {}
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
std::shared_ptr<Image> LoadImage(const std::string& filePath);
bool ReadFile(const char* pFileName, std::string& outFile);
void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);