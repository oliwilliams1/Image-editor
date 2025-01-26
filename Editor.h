#pragma once
#include <iostream>
#include <GL/glew.h>
#include <imgui.h>
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
	void RenderUI();

	GLuint mainTexture;

private:
	void SetupQuad();
	Shader* shader;
	Image currentImage;

	GLuint u_InputImageLoc;
	GLuint editorFBO, quadVAO, quadVBO;

	void SetupUBO();
	GLuint UBO;
};