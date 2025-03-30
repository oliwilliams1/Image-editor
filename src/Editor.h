#pragma once
#include <iostream>
#include <memory>
#include <GL/glew.h>
#include <imgui.h>
#include "utils.h"
#include "Shader.h"

class Editor
{
public:
	Editor();
	~Editor();

	void Initialize();
	void SetImage(std::shared_ptr<Image> imagePtr);
	void Render();
	void RenderUI();
	void SaveImage(std::shared_ptr<Image> image);
	std::shared_ptr<Image> currentImage;

	GLuint logoImageLoc = -1;

private:
	void SetupQuad();
	Shader* shader;

	GLuint u_InputImageLoc;
	GLuint editorFBO, quadVAO, quadVBO;

	int outputJpgQuality = 90;

	void ShowFloatAsCheckbox(float* value, const char* label, bool updSSBO = false);
	void ImGuiFloatSlider(const char* label, float* value, float min, float max, float step, bool updSSBO = false);
	void ImGuiVec3Slider(const char* label, glm::vec3* value, float min, float max, float step, bool updSSBO = false);

	void SetupUBO();
	void UpdateUBO();
	GLuint UBO;
	bool needsUBOUpdate = false;

	void SetupMaskSSBO();
	void UpdateMaskSSBO();
	GLuint maskSSBO;
	bool needSSBOUpdate = false;

	GLuint u_LogoImageLoc;
};