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
	void SaveImage();

	GLuint mainTexture;

private:
	void SetupQuad();
	Shader* shader;
	std::shared_ptr<Image> currentImage;

	GLuint u_InputImageLoc;
	GLuint editorFBO, quadVAO, quadVBO;

	void ShowFloatAsCheckbox(float* value, const char* label);
	void ImGuiFloatSlider(const char* label, float* value, float min, float max, float step);
	void ImGuiVec3Slider(const char* label, glm::vec3* value, float min, float max, float step);

	void SetupUBO();
	GLuint UBO;
	bool needsUpdate = false;
};