#include "App.h"
#include "utils.h"

// static, only this translation unit, cant be bothered to make this code somewhere else
static bool dragging = false;
static double offsetX, offsetY;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			dragging = true;
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			offsetX = xpos;
			offsetY = ypos;
		}
		else if (action == GLFW_RELEASE) {
			dragging = false;
		}
	}
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	if (dragging) {
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		int x, y;
		glfwGetWindowPos(window, &x, &y);

		if (ypos > 32) return;
		if (xpos < 175 || xpos > width - 175) return;
		// Calculate new window position
		int newX = x + static_cast<int>(xpos - offsetX);
		int newY = y + static_cast<int>(ypos - offsetY);

		glfwSetWindowPos(window, newX, newY);
	}
}

App::App() 
{
	smallWindowWidth = 1600;
	smallWindowHeight = 900;
	windowWidth = smallWindowWidth;
	windowHeight = smallWindowHeight;
	InitWindow();
	InitImGui();
	SetupImGuiStyle();
}

void App::InitWindow() 
{
	if (!glfwInit()) 
	{
		std::cout << stderr << "Failed to initialize GLFW" << std::endl;
		return;
	}

	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

	// Create a window with OpenGL
	window = glfwCreateWindow(windowWidth, windowHeight, "Planet Renderer", nullptr, nullptr);
	if (!window) 
	{
		std::cout << stderr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	// Calculate the center position
	int windowWidth, windowHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);
	int posX = (mode->width - windowWidth) / 2;
	int posY = (mode->height - windowHeight) / 2;

	screenSizeX = mode->width;
	screenSizeY = mode->height;

	// Set the window position to the center
	glfwSetWindowPos(window, posX, posY);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	glfwMakeContextCurrent(window);

	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);

	GLenum err = glewInit();
	if (err != GLEW_OK) 
	{
		std::cout << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
		return;
	}

	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
}

void App::InitImGui() 
{
	// Init ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

void App::Mainloop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);

		RenderUI();

		glfwSwapBuffers(window);
	}
}

void App::RenderUI()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Top Bar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::SetWindowSize(ImVec2(windowWidth, 40));
	ImGui::SetWindowPos(ImVec2(0, 0));

	if (ImGui::Button("File")) fileWindowOpen = !fileWindowOpen;
	ImGui::SameLine();
	ImGui::Button("Edit");
	ImGui::SameLine();
	ImGui::Button("View");
	ImGui::SameLine();
	ImGui::Button("Help");

	ImGui::SameLine(ImGui::GetWindowWidth() * 0.5f - ImGui::CalcTextSize("Sable Image Editor").x * 0.5f);
	ImGui::Text("Sable Image Editor");

	ImGui::SameLine(ImGui::GetWindowWidth() - 96);
	if (ImGui::Button("_", ImVec2(24, 24)))
	{
		glfwIconifyWindow(window);
	}

	ImGui::SameLine();

	if (windowWidth == smallWindowWidth)
	{
		if (ImGui::Button("+", ImVec2(24, 24))) {
			windowWidth = screenSizeX;
			windowHeight = screenSizeY;
			glfwSetWindowPos(window, 0, 0);
			glfwSetWindowSize(window, windowWidth, windowHeight);
		}
	}
	else
	{
		if (ImGui::Button("-", ImVec2(24, 24)))
		{
			windowWidth = smallWindowWidth;
			windowHeight = smallWindowHeight;
			glfwSetWindowPos(window, 0, 0);
			glfwSetWindowSize(window, windowWidth, windowHeight);
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("x", ImVec2(24, 24)))
	{
		glfwSetWindowShouldClose(window, true);
	}

	ImGui::End();

	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::SetWindowSize(ImVec2(windowWidth - 320, windowHeight - 256 - 40));
	ImGui::SetWindowPos(ImVec2(0, 40));
	ImGui::End();

	ImGui::Begin("Image Settings", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::SetWindowSize(ImVec2(320, windowHeight - 256 - 40));
	ImGui::SetWindowPos(ImVec2(windowWidth - 320, 40));
	ImGui::End();

	ImGui::Begin("Photos", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::SetWindowSize(ImVec2(windowWidth, 256));
	ImGui::SetWindowPos(ImVec2(0, windowHeight - 256));
	ImGui::Text("Gallery");
	ImGui::End();

	if (fileWindowOpen) {
		ImGui::Begin("File window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::SetWindowSize(ImVec2(0, 0));
		ImGui::SetWindowPos(ImVec2(4, 32));
		if (ImGui::Selectable("Open folder"))
		{
			fileWindowOpen = false;
			NFD_Init();

			nfdu8char_t* outPath = nullptr;

			nfdresult_t result = NFD_PickFolderU8(&outPath, nullptr);

			if (result == NFD_OKAY) 
{
				std::cout << "User selected folder: " << outPath << std::endl;

				NFD_FreePathU8(outPath);
			}
			else if (result == NFD_CANCEL) 
			{
				std::cout << "User pressed cancel" << std::endl;
			}
			else 
{
				std::cerr << "Error: " << NFD_GetError() << std::endl;
			}

			NFD_Quit();
		}
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

App::~App() 
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
}