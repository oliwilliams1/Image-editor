#include "App.h"

// static, only this translation unit, cant be bothered to make this code somewhere else
static bool dragging = false;
static double offsetX, offsetY;

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) 
	{
		if (action == GLFW_PRESS) 
		{
			dragging = true;
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			offsetX = xpos;
			offsetY = ypos;
		}
		else if (action == GLFW_RELEASE) 
		{
			dragging = false;
		}
	}
}

static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
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

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
	SetupImGuiStyle();
}

void App::Mainloop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RenderUI();

		glfwSwapBuffers(window);
	}
}

void App::OpenFolderContents(const std::string& folderPath) 
{
	std::vector<std::string> supportedFileTypes = { ".png", ".jpg", "jpeg", ".bmp" };
	std::vector<std::string> files;

	std::filesystem::path path(folderPath);

	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		if (entry.is_regular_file()) {
			std::string fileName = entry.path().filename().string();
			std::string extension = entry.path().extension().string();

			// Check if the file type is supported
			if (std::find(supportedFileTypes.begin(), supportedFileTypes.end(), extension) != supportedFileTypes.end()) {
				
				std::filesystem::path filePath = entry.path();
				files.push_back(filePath.string());
			}
		}
	}

	std::lock_guard<std::mutex> lock(imageMutex);
	wishImagesAmnt = files.size();

	for (const auto& filePath: files) 
	{
		images.push_back(LoadImage(filePath));
		std::cout << "Loaded image: " << filePath << std::endl;
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

	ImGui::Begin("Gallery", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::SetWindowSize(ImVec2(windowWidth, 256));
	ImGui::SetWindowPos(ImVec2(0, windowHeight - 256));

	ImGui::Text("Gallery");

	for (int i = 0; i < images.size(); i++) 
	{
		ImGui::Image((ImTextureID)(intptr_t)images[i].textureID, ImVec2(64, 64), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	if (images.size() != wishImagesAmnt)
	{
		float progressBarHeight = 24.0f;
		std::string label = "Loading images (" + std::to_string(images.size()) + "/" + std::to_string(wishImagesAmnt) + ")";

		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - progressBarHeight - ImGui::GetStyle().WindowPadding.y);

		ImGui::ProgressBar((float)images.size() / (float)wishImagesAmnt, ImVec2(windowWidth - 12, progressBarHeight), label.c_str());
	} 
	else 
	{
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 15 - ImGui::GetStyle().WindowPadding.y);

		ImGui::Text("Loaded images %i", images.size());
	}

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

				std::string folderPath = outPath;
				OpenFolderContents(folderPath);

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
	for (int i = 0; i < images.size(); i++)
	{
		glDeleteTextures(1, &images[i].textureID);
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
}