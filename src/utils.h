#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include <GL/glew.h>
#include <glm/vec3.hpp>

using SBmask = int;

constexpr SBmask SABLE_NO_MASK = 0x000000;
constexpr SBmask SABLE_COLOUR_MASK = 0x000001;
constexpr SBmask SABLE_LUMINANCE_MASK = 0x000002;

struct ImageEditData
{
	glm::vec3 avgColour = glm::vec3(0.0f);
	float padding = 1.0f;
	glm::vec3 AWB_ScalingFactors = glm::vec3(1.0f);
	float padding2 = 1.0f;

	unsigned int numMasks = 0;

	float gamma = 2.2f; // [0.2, 5.0]

	float exposure = 0.0f; // [-5, 5] working ev
	float reinhard = 0.0f; // [0 or 1] switch

	float colTemp = 0.0f; // [-1.0, 1.0]
	float colTint = 0.0f; // [-1.0, 1.0]
	
	float hue = 0.0f; // [-180, 180] degrees
	float saturation = 1.0f; // [0, 5]
	float invert = 0.0f; // [0 or 1] switch

	float u_ApplyAwb = 0.0f; // [0 or 1] switch

	float shadows = 0.0f; // [-1, 1]
	float highlights = 0.0f; // [-1, 1]

	float contrast = 0.5f;

	float angle = 0.0f; // [-180, 180] degrees
};

struct MaskEditData
{
	glm::vec3 colourMask = glm::vec3(1.0f);
	float colourMaskThreshold = 0.5f;

	float maskType = 0.0f;

	float luminanceMaskLower = 0.0f;
	float luminanceMaskUpper = 1.0f;
	float luminanceMaskInvert = 0.0f;

	float exposure = 0.0f;
	float reinhard = 0.0f;

	float contrast = 0.5f;

	float colTemp = 0.0f;
	float colTint = 0.0f;

	float hue = 0.0f;
	float saturation = 1.0f;
	float invert = 0.0f;

	float shadows = 0.0f;
	float highlights = 0.0f;

	float viewMask = 0.0f;
};

struct Mask
{
	SBmask maskType = SABLE_NO_MASK;
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