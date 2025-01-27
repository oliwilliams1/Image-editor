#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include <GL/glew.h>
#include <glm/vec3.hpp>

using SBmask = int;

constexpr SBmask SABLE_COLOUR_MASK = 0x000000;
constexpr SBmask SABLE_LUMINANCE_MASK = 0x000001;

struct ImageEditData
{
	glm::vec3 avgColour;
	float p1 = 1.0f;
	glm::vec3 AWB_ScalingFactors;
	float p2 = 1.0f;

	float gamma; // [0.2, 5.0]

	float exposure; // [-5, 5] working ev
	float reinhard; // [0 or 1] switch

	float colTemp; // [-1.0, 1.0]
	float colTint; // [-1.0, 1.0]
	
	float hue; // [-180, 180] degrees
	float saturation; // [0, 5]
	float invert; // [0 or 1] switch

	float u_ApplyAwb; // [0 or 1] switch

	float shadows; // [-1, 1]
	float highlights; // [-1, 1]

	ImageEditData() : exposure(0.0f), reinhard(0.0f), gamma(2.2f), 
		colTemp(0.0f), colTint(0.0f), hue(0.0f), saturation(1.0f), 
		invert(0.0f), avgColour(0.0f), u_ApplyAwb(0.0f), AWB_ScalingFactors(1.0f), 
		shadows(0.0f), highlights(0.0f) {}
};

struct MaskEditData
{
	float exposure;
	float reinhard;

	float colTemp;
	float colTint;

	float hue;
	float saturation;
	float invert;

	float shadows;
	float highlights;
};

struct Mask
{
	SBmask maskType;
	std::string name;
	MaskEditData editData;
};

struct Image
{
	GLuint textureInID;
	GLuint FBO, textureOutID;
	std::string filePath;
	int width, height, channels;
	ImageEditData editData;
	std::vector<Mask> masks;
};

glm::vec3 GetAvgColour(GLuint textureID, GLint maxLevel);
void SetupImGuiStyle();
std::shared_ptr<Image> LoadImage(const std::string& filePath);
bool ReadFile(const char* pFileName, std::string& outFile);
void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);