#include "Editor.h"

Editor::Editor() {};

void Editor::Initialize() {
	SetupQuad();

	colTempS = 6500.0f;
	colTempT = 6500.0f;

	glGenFramebuffers(1, &editorFBO);

	shader = new Shader("shaders/editor.vert", "shaders/editor.frag");
	
	u_InputImageLoc = glGetUniformLocation(shader->shaderProgram, "u_InputImage");
	u_ColTempSLoc = glGetUniformLocation(shader->shaderProgram, "u_Ct_s");
	u_ColTempTLoc = glGetUniformLocation(shader->shaderProgram, "u_Ct_t");

	shader->use();
	glUniform1f(u_ColTempSLoc, colTempS);
	glUniform1f(u_ColTempTLoc, colTempT);
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

void Editor::SetImage(Image image)
{
	this->currentImage = image;
	glBindFramebuffer(GL_FRAMEBUFFER, editorFBO);

	if (mainTexture != 0)
	{
		glDeleteTextures(1, &mainTexture);
		mainTexture = 0;
	}

	glGenTextures(1, &mainTexture);
	glBindTexture(GL_TEXTURE_2D, mainTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, image.width, image.height, 0, GL_RGBA, GL_FLOAT, nullptr);
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
}

void Editor::Render() 
{
	glBindFramebuffer(GL_FRAMEBUFFER, editorFBO);
	glViewport(0, 0, currentImage.width, currentImage.height);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mainTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, currentImage.textureID);

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
	ImGui::Text("Colour temperature");
	if (ImGui::DragFloat("Colour temp start", &colTempS, 10.0f, 2000.0f, 10000.0f)) glUniform1f(u_ColTempSLoc, colTempS);
	if (ImGui::DragFloat("Colour temp target", &colTempT, 10.0f, 2000.0f, 10000.0f)) glUniform1f(u_ColTempTLoc, colTempT);
}

Editor::~Editor() 
{
	glDeleteFramebuffers(1, &editorFBO);
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
}