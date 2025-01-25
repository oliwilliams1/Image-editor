#pragma once

#include <iostream>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <nfd.h>

struct Image
{
	GLuint textureID;
	std::string filePath;
	int width, height, channels;
};

class App
{
public:
	App();
	void Mainloop();
	~App();

private:
	GLFWwindow* window;
	std::mutex imageMutex;
	std::vector<Image> images;
	int wishImagesAmnt;

	bool fileWindowOpen;
	int screenSizeX, screenSizeY;
	int windowWidth, windowHeight;
	int smallWindowWidth, smallWindowHeight;
	void InitWindow();
	void InitImGui();
	void RenderUI();
	void LoadImage(const std::string& filePath);
	void OpenFolderContents(const std::string& folderPath);
};