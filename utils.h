#pragma once
#include <string>
#include <iostream>
#include <GL/glew.h>

struct Image
{
	GLuint textureID;
	std::string filePath;
	int width, height, channels;
};

void SetupImGuiStyle();
Image LoadImage(const std::string& filePath);