#pragma once
#include <iostream>
#include <memory>
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
	void SetImage(std::shared_ptr<Image> imagePtr);
	void Render();
	void RenderUI();

	GLuint mainTexture;

private:
	void SetupQuad();
	Shader* shader;
	std::shared_ptr<Image> currentImage;

	GLuint u_InputImageLoc;
	GLuint editorFBO, quadVAO, quadVBO;

	void SetupUBO();
	GLuint UBO;
};