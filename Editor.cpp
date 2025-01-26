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

void Editor::RenderUI()
{
	if (currentImage == nullptr) return;
	bool needsUpdate = false;

	ImGui::SeparatorText("Image data");
	if (ImGui::DragFloat("Gamma", &currentImage->editData.gamma, 0.1f, 0.2f, 5.0f)) needsUpdate = true;

	ImGui::SeparatorText("Adjustments");
	if (ImGui::DragFloat("Exposure", &currentImage->editData.exposure, 1.0f, 0.0f, 1.0f)) needsUpdate = true;
	if (ImGui::DragFloat("Gamut map", &currentImage->editData.gamutMap, 1.0f, 0.0f, 1.0f)) needsUpdate = true;
	if (ImGui::DragFloat("Tonemap", &currentImage->editData.reinhard, 1.0f, 0.0f, 1.0f)) needsUpdate = true;

	ImGui::SeparatorText("Temperature");
	if (ImGui::DragFloat("Col Temp S", &currentImage->editData.colTempS, 10.0f, 2000.0f, 10000.0f)) needsUpdate = true;
	if (ImGui::DragFloat("Col Temp T", &currentImage->editData.colTempT, 10.0f, 2000.0f, 10000.0f)) needsUpdate = true;

	ImGui::SeparatorText("Colour");
	if (ImGui::DragFloat3("Colour Balance", &currentImage->editData.colBalance.x, 0.05f, -1.0f, 1.0f)) needsUpdate = true;
	if (ImGui::DragFloat("Preserve Luma", &currentImage->editData.keepLumaColBalance, 1.0f, 0.0f, 1.0f)) needsUpdate = true;
	if (ImGui::DragFloat("Hue", &currentImage->editData.hue, 1.0f, -180.0f, 180.0f)) needsUpdate = true;
	if (ImGui::DragFloat("Saturation", &currentImage->editData.saturation, 0.05f, 0.0f, 5.0f)) needsUpdate = true;
	if (ImGui::DragFloat("Invert luminance", &currentImage->editData.invert, 1.0f, 0.0f, 1.0f)) needsUpdate = true;
	
	glm::vec3 avgColour = glm::vec3(currentImage->editData.avgColour); // Create a copy
	ImVec4 imAvgColour = ImVec4(avgColour.x, avgColour.y, avgColour.z, 1.0f);

	ImGui::ColorButton("Average colour", imAvgColour, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoOptions);
	if (ImGui::DragFloat("Apply AWB", &currentImage->editData.u_ApplyAwb , 1.0f, 0.0f, 1.0f)) needsUpdate = true;

	if (needsUpdate)
	{
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