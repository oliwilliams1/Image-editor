#include "Editor.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

Editor::Editor() {};

void Editor::Initialize() {
	SetupQuad();
	SetupUBO();
	SetupMaskSSBO();

	glGenFramebuffers(1, &editorFBO);

	shader = new Shader("shaders/editor.vert", "shaders/editor.frag");
	
	u_InputImageLoc = glGetUniformLocation(shader->shaderProgram, "u_InputImage");
	u_LogoImageLoc = glGetUniformLocation(shader->shaderProgram, "u_LogoImage");

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

void Editor::SetupMaskSSBO()
{
	glGenBuffers(1, &maskSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, maskSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(MaskEditData), nullptr, GL_DYNAMIC_DRAW); // prob dont need this but cant be bothered finding out
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, maskSSBO);
}

void Editor::SetImage(std::shared_ptr<Image> imagePtr)
{
	this->currentImage = imagePtr;

	if (imagePtr == nullptr) return;

	glBindFramebuffer(GL_FRAMEBUFFER, editorFBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, imagePtr->textureOutID, 0);

	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
	{
		std::cerr << "Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	UpdateUBO();
	UpdateMaskSSBO();	
}

void Editor::Render() 
{
	if (currentImage == nullptr) return;
	glBindFramebuffer(GL_FRAMEBUFFER, editorFBO);
	glViewport(0, 0, currentImage->width, currentImage->height);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, currentImage->textureOutID);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, currentImage->textureInID);

	if (logoImageLoc != -1)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, logoImageLoc);
	}

	glClear(GL_COLOR_BUFFER_BIT);

	shader->use();
	glUniform1i(u_InputImageLoc, 1);

	if (logoImageLoc != -1)
	{
		glUniform1i(u_LogoImageLoc, 2);
	}

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Editor::SaveImage(std::shared_ptr<Image> image)
{
	SetImage(image);
	Render();

	unsigned char* data = new unsigned char[currentImage->width * currentImage->height * currentImage->channels];

	GLenum format = GL_RGB;
	if (currentImage->channels == 4)
	{
		format = GL_RGBA;
	}

	glBindTexture(GL_TEXTURE_2D, currentImage->textureOutID);
	glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, data);

	std::string fileName = currentImage->filePath;
	size_t dotIndex = fileName.find_last_of('.');
	if (dotIndex != std::string::npos)
	{
		fileName = fileName.substr(0, dotIndex);
	}

	fileName += ".jpg";

	std::cout << "Saving image: " << fileName << std::endl;
	stbi_write_jpg(fileName.c_str(), currentImage->width, currentImage->height, currentImage->channels, data, outputJpgQuality);
}

void Editor::ShowFloatAsCheckbox(float* value, const char* label, bool updSSBO)
{
	bool checkboxValue = *value != 0.0f;

	if (ImGui::Checkbox(label, &checkboxValue))
	{
		*value = checkboxValue ? 1.0f : 0.0f;
		needsUBOUpdate = true;
		if (updSSBO) needSSBOUpdate = true;
	}
}

void Editor::ImGuiFloatSlider(const char* label, float* value, float min, float max, float step, bool updSSBO)
{
	ImGui::Text(label);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(-FLT_MIN);
	if (ImGui::SliderFloat(label, value, min, max, "%.2f"))
	{
		needsUBOUpdate = true;
		if (updSSBO) needSSBOUpdate = true;
	}
}

void Editor::ImGuiVec3Slider(const char* label, glm::vec3* value, float min, float max, float step, bool updSSBO)
{
	ImGui::Text(label);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(-FLT_MIN);
	if (ImGui::SliderFloat3(label, &value->x, min, max, "%.2f"))
	{
		needsUBOUpdate = true;
		if (updSSBO) needSSBOUpdate = true;
	}
}

void Editor::RenderUI() {
	if (currentImage == nullptr) return;

	ImGui::BeginTabBar("Editor tab bar");

	if (ImGui::BeginTabItem("Image")) {
		// Export Section
		ImGui::SeparatorText("Export");
		if (ImGui::Button("Save")) SaveImage(currentImage);
		ImGui::SliderInt("JPG Quality", &outputJpgQuality, 0, 100);

		// Image Data Section
		ImGui::SeparatorText("Image Data");
		ImGui::Text("Resolution: %ix%i, channels: %i", currentImage->width, currentImage->height, currentImage->channels);
		ImGuiFloatSlider("Gamma", &currentImage->editData.gamma, 0.2f, 5.0f, 0.1f);

		ImGui::SeparatorText("Adjustments");
		ImGuiFloatSlider("Exposure", &currentImage->editData.exposure, -5.0f, 5.0f, 0.1f);
		ShowFloatAsCheckbox(&currentImage->editData.reinhard, "Tonemap");
		ImGuiFloatSlider("Contrast", &currentImage->editData.contrast, 0.0f, 1.0f, 0.05f);
		ImGuiFloatSlider("Shadows", &currentImage->editData.shadows, -1.0f, 1.0f, 0.05f);
		ImGuiFloatSlider("Highlights", &currentImage->editData.highlights, -1.0f, 1.0f, 0.05f);

		// Temperature Section
		ImGui::SeparatorText("Temperature");
		ShowFloatAsCheckbox(&currentImage->editData.u_ApplyAwb, "Auto White Balance");
		ImGuiFloatSlider("Color Temp", &currentImage->editData.colTemp, -1.0f, 1.0f, 0.05f);
		ImGuiFloatSlider("Color Tint", &currentImage->editData.colTint, -1.0f, 1.0f, 0.05f);

		// Color Section
		ImGui::SeparatorText("Colour");
		ImGuiFloatSlider("Hue", &currentImage->editData.hue, -180.0f, 180.0f, 1.0f);
		ImGuiFloatSlider("Saturation", &currentImage->editData.saturation, 0.0f, 5.0f, 0.05f);
		ShowFloatAsCheckbox(&currentImage->editData.invert, "Invert Luminance");

		// Average Color Button
		glm::vec3 avgColour = glm::vec3(currentImage->editData.avgColour); // Create a copy
		ImVec4 imAvgColour = ImVec4(avgColour.x, avgColour.y, avgColour.z, 1.0f);
		ImGui::ColorButton("Average Colour", imAvgColour, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoOptions);
		ImGui::EndTabItem();

		// Image at bottom of 360 image
		ShowFloatAsCheckbox(&currentImage->editData.logo360, "Place Logo at bottom");
		ImGuiFloatSlider("Logo Radius", &currentImage->editData.logo360Radius, 0.0f, 1.0f, 0.05f);
	}

	if (ImGui::BeginTabItem("Masks"))
	{
		if (ImGui::Button("Add Mask"))
		{
			Mask mask;
			mask.name = "Mask " + std::to_string(currentImage->masks.size() + 1);
			currentImage->masks.push_back(mask);
			
			needSSBOUpdate = true;
		}

		int deleteMaskIndex = -1;

		if (currentImage->masks.size() != 0)
		{
			ImGui::BeginTabBar("Masks tab bar");
			for (int i = 0; i < currentImage->masks.size(); i++)
			{
				if (ImGui::BeginTabItem(currentImage->masks[i].name.c_str()))
				{
					ImGui::Text(currentImage->masks[i].name.c_str());
					if (ImGui::Button("Delete Mask"))
					{
						needSSBOUpdate = true;
						deleteMaskIndex = i;
					}
					ImGui::EndTabItem();

					if (currentImage->masks[i].maskType != SABLE_NO_MASK)
					{
						ShowFloatAsCheckbox(&currentImage->masks[i].editData.viewMask, "Preview mask", true);
					}

					static const char* maskTypes[] = { "Choose a mask type", "Colour", "Luminance" };

					ImGui::Text("Mask Type");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(-FLT_MIN);
					if (ImGui::Combo("Mask Type", &currentImage->masks[i].maskType, maskTypes, IM_ARRAYSIZE(maskTypes)))
					{
						needSSBOUpdate = true;
					}

					if (currentImage->masks[i].maskType == SABLE_COLOUR_MASK)
					{
						if (ImGui::ColorPicker3("Mask Colour", &currentImage->masks[i].editData.colourMask.x))
						{
							needSSBOUpdate = true;
						}
						ImGuiFloatSlider("Colour mask threshold", &currentImage->masks[i].editData.colourMaskThreshold, 0.0f, 1.0f, 0.05f, true);
					}

					if (currentImage->masks[i].maskType == SABLE_LUMINANCE_MASK)
					{
						ImGuiFloatSlider("Luminance mask lower", &currentImage->masks[i].editData.luminanceMaskLower,  0.0f, 1.0f, 0.05f, true);
						ImGuiFloatSlider("Luminance mask upper", &currentImage->masks[i].editData.luminanceMaskUpper , 0.0f, 1.0f, 0.05f, true);
						ShowFloatAsCheckbox(&currentImage->masks[i].editData.luminanceMaskInvert, "Invert luminance mask", true);
					}

					// Adjustments
					ImGui::SeparatorText("Adjustments");
					ImGuiFloatSlider("Exposure", &currentImage->masks[i].editData.exposure, -5.0f, 5.0f, 0.1f, true);
					ShowFloatAsCheckbox(&currentImage->masks[i].editData.reinhard, "Tonemap", true);
					ImGuiFloatSlider("Contrast", &currentImage->masks[i].editData.contrast, 0.0f, 1.0f, 0.05f, true);
					ImGuiFloatSlider("Shadows", &currentImage->masks[i].editData.shadows, -1.0f, 1.0f, 0.05f, true);
					ImGuiFloatSlider("Highlights", &currentImage->masks[i].editData.highlights, -1.0f, 1.0f, 0.05f, true);

					// Temperature Section
					ImGui::SeparatorText("Temperature");
					ImGuiFloatSlider("Color Temp", &currentImage->masks[i].editData.colTemp, -1.0f, 1.0f, 0.05f, true);
					ImGuiFloatSlider("Color Tint", &currentImage->masks[i].editData.colTint, -1.0f, 1.0f, 0.05f, true);

					// Color Section
					ImGui::SeparatorText("Colour");
					ImGuiFloatSlider("Hue", &currentImage->masks[i].editData.hue, -180.0f, 180.0f, 1.0f, true);
					ImGuiFloatSlider("Saturation", &currentImage->masks[i].editData.saturation, 0.0f, 5.0f, 0.05f, true);
					ShowFloatAsCheckbox(&currentImage->masks[i].editData.invert, "Invert Luminance", true);
				}
			}
			ImGui::EndTabBar();
		}

		if (deleteMaskIndex != -1) 
		{
			currentImage->masks.erase(currentImage->masks.begin() + deleteMaskIndex);
		}

		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();

	// Update Data if Needed
	if (needsUBOUpdate) {
		UpdateUBO();
	}

	if (needSSBOUpdate) {
		UpdateUBO(); // <-- Num masks is stored in here, so it needs to be updated for gpu to not kill itself
		UpdateMaskSSBO();
	}
}

void Editor::UpdateUBO()
{
	currentImage->editData.numMasks = currentImage->masks.size();

	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ImageEditData), &currentImage->editData, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	needsUBOUpdate = false;
}

void Editor::UpdateMaskSSBO()
{
	for (int i = 0; i < currentImage->masks.size(); i++)
	{
		currentImage->masks[i].editData.maskType = currentImage->masks[i].maskType;
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, maskSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(MaskEditData) * currentImage->masks.size(), nullptr, GL_DYNAMIC_DRAW);

	for (int i = 0; i < currentImage->masks.size(); i++)
	{
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, i * sizeof(MaskEditData), sizeof(MaskEditData), &currentImage->masks[i].editData);
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	needSSBOUpdate = false;
}

Editor::~Editor() 
{
	glDeleteFramebuffers(1, &editorFBO);
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
}