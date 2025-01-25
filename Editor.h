#pragma once
#include <iostream>
#include <GL/glew.h>
#include "utils.h"

class Editor
{
public:
	Editor();
	~Editor();

	void Initialize();
	void SetImage(Image image);
	GLuint mainTexture;

private:
	void SetupQuad();

	GLuint editorFBO, quadVAO, quadVBO;
};