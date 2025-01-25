#pragma once

#include <iostream>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <nfd.h>

#include "utils.h"

class App
{
public:
	App();
	void Mainloop();
	~App();

private:
	GLFWwindow* window;
	std::vector<std::string> imagePathQueue;
	std::vector<Image> images;
	int wishImagesAmnt;

	int selectedImageIndex;

	bool fileWindowOpen;
	int screenSizeX, screenSizeY;
	int windowWidth, windowHeight;
	int smallWindowWidth, smallWindowHeight;
	void InitWindow();
	void InitImGui();
	void RenderUI();
	void OpenFolderContents(const std::string& folderPath);
};