#pragma once
#include <iostream>
#include <GL/glew.h>
#include "utils.h"
#include "shader.h"

class Editor
{
public:
	Editor();
	~Editor();

	void Initialize();
	void SetImage(Image image);
	void Render();
	GLuint mainTexture;

private:
	void SetupQuad();
	Shader* shader;
	Image currentImage;

	GLuint u_InputImageLoc;
	GLuint editorFBO, quadVAO, quadVBO;
};