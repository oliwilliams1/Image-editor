#include "Editor.h"

Editor::Editor() {};

void Editor::Initialize() {
	SetupQuad();
	SetupUBO();

	glGenFramebuffers(1, &editorFBO);

	shader = new Shader("shaders/editor.vert", "shaders/editor.frag");
	
	u_InputImageLoc = glGetUniformLocation(shader->shaderProgram, "u_InputImage");

	shader->use();
}

void Editor::SetupQuad() 
{
	float vertices[18] = {
		-1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);

	glBindVertexArray(quadVAO);

	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

void Editor::SetupUBO() 
{
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ImageEditData), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);
}

void Editor::SetImage(std::shared_ptr<Image> imagePtr)
{
	this->currentImage = imagePtr;
	glBindFramebuffer(GL_FRAMEBUFFER, editorFBO);

	if (mainTexture != 0)
	{
		glDeleteTextures(1, &mainTexture);
		mainTexture = 0;
	}

	glGenTextures(1, &mainTexture);
	glBindTexture(GL_TEXTURE_2D, mainTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, imagePtr->width, imagePtr->height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mainTexture, 0);

	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
	{
		std::cerr << "Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ImageEditData), &imagePtr->editData, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Editor::Render() 
{
	glBindFramebuffer(GL_FRAMEBUFFER, editorFBO);
	glViewport(0, 0, currentImage->width, currentImage->height);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mainTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, currentImage->textureID);

	glClear(GL_COLOR_BUFFER_BIT);

	shader->use();
	glUniform1i(u_InputImageLoc, 1);

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Editor::ShowFloatAsCheckbox(float* value, const char* label)
{
	bool checkboxValue = *value != 0.0f;

	if (ImGui::Checkbox(label, &checkboxValue))
	{
		*value = checkboxValue ? 1.0f : 0.0f;
		needsUpdate = true;
	}
}

void Editor::ImGuiFloatSlider(const char* label, float* value, float min, float max, float step)
{
	ImGui::Text(label);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(-FLT_MIN);
	if (ImGui::SliderFloat(label, value, min, max, "%.2f"))
	{
		needsUpdate = true;
	}
}

void Editor::ImGuiVec3Slider(const char* label, glm::vec3* value, float min, float max, float step)
{
	ImGui::Text(label);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(-FLT_MIN);
	if (ImGui::SliderFloat3(label, &value->x, min, max, "%.2f"))
	{
		needsUpdate = true;
	}
}

void Editor::RenderUI() {
	if (currentImage == nullptr) return;

	// Image Data Section
	ImGui::SeparatorText("Image Data");
	ImGuiFloatSlider("Gamma", &currentImage->editData.gamma, 0.2f, 5.0f, 0.1f);

	ImGui::SeparatorText("Adjustments");
	ImGuiFloatSlider("Exposure", &currentImage->editData.exposure, -5.0f, 5.0f, 0.1f);
	ShowFloatAsCheckbox(&currentImage->editData.gamutMap, "Gamut Map");
	ShowFloatAsCheckbox(&currentImage->editData.reinhard, "Tonemap");

	// Temperature Section
	ImGui::SeparatorText("Temperature");
	ShowFloatAsCheckbox(&currentImage->editData.u_ApplyAwb, "Auto White Balance");
	ImGuiFloatSlider("Color Temp S", &currentImage->editData.colTempS, 2000.0f, 10000.0f, 10.0f);
	ImGuiFloatSlider("Color Temp T", &currentImage->editData.colTempT, 2000.0f, 10000.0f, 10.0f);

	// Color Section
	ImGui::SeparatorText("Colour");
	ImGuiFloatSlider("Hue", &currentImage->editData.hue, -180.0f, 180.0f, 1.0f);
	ImGuiFloatSlider("Saturation", &currentImage->editData.saturation, 0.0f, 5.0f, 0.05f);
	ShowFloatAsCheckbox(&currentImage->editData.invert, "Invert Luminance");

	// Average Color Button
	glm::vec3 avgColour = glm::vec3(currentImage->editData.avgColour); // Create a copy
	ImVec4 imAvgColour = ImVec4(avgColour.x, avgColour.y, avgColour.z, 1.0f);
	ImGui::ColorButton("Average Colour", imAvgColour, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoOptions);

	// Update Data if Needed
	if (needsUpdate) {
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(ImageEditData), &currentImage->editData, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}

Editor::~Editor() 
{
	glDeleteFramebuffers(1, &editorFBO);
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
}