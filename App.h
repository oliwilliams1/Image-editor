#pragma once

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <nfd.h>

class App
{
public:
	App();
	void Mainloop();
	~App();

private:
	GLFWwindow* window;

	bool fileWindowOpen;
	int screenSizeX, screenSizeY;
	int windowWidth, windowHeight;
	int smallWindowWidth, smallWindowHeight;
	void InitWindow();
	void InitImGui();
	void RenderUI();
};