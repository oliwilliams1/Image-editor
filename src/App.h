#pragma once

#include <iostream>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <array>
#include <memory>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Editor.h"
#include "utils.h"

class App
{
public:
	App();
	void Mainloop();
	int windowWidth, windowHeight;
	~App();

private:
	Editor editor;

	GLFWwindow* window;
	std::vector<std::string> imagePathQueue;
	std::vector<std::shared_ptr<Image>> images;
	std::vector<std::shared_ptr<Image>> imageSaveQueue;
	int wishImagesAmnt = 0;
	int saveImagesAmnt = 0;

	int selectedImageIndex;

	bool fileWindowOpen = false;
	int screenSizeX, screenSizeY;
	int smallWindowWidth, smallWindowHeight;
	void InitWindow();
	void InitImGui();
	void RenderUI();
	void OpenFolderContents(const std::string& folderPath);
};