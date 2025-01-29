#include "App.h"

// static, only this translation unit, cant be bothered to make this code somewhere else
static bool dragging = false;
static double offsetX, offsetY;
static int needsRender = 3; // initialize run through + two buffers to fill in

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) 
{
	needsRender = 3; // 1 frame for imgui updating data, 2 frames to fill in both buffers
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

static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) 
{
	needsRender = 3; // 1 frame for imgui updating data, 2 frames to fill in both buffers
	if (dragging) 
	{
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		int x, y;
		glfwGetWindowPos(window, &x, &y);

		if (ypos > 45) return;
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

	selectedImageIndex = -1;

	InitWindow();
	InitImGui();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	editor.Initialize();
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
	SetupImGuiStyle();
}

void App::Mainloop()
{
	while (!glfwWindowShouldClose(window))
	{
		if (imagePathQueue.size() != 0)
		{
			std::shared_ptr<Image> image = LoadImage(imagePathQueue.front());
			images.push_back(image);
			std::cout << "Loaded image: " << imagePathQueue.front() << std::endl;
			imagePathQueue.erase(imagePathQueue.begin());

			editor.SetImage(image);
			editor.Render();

			needsRender = 2; // 2 buffers
		}

		if (imageSaveQueue.size() != 0)
		{
			std::shared_ptr<Image> image = imageSaveQueue.front();

			editor.SaveImage(image);

			glDeleteTextures(1, &image->textureInID);
			glDeleteTextures(1, &image->textureOutID);

			imageSaveQueue.erase(imageSaveQueue.begin());

			selectedImageIndex = -1;
			editor.SetImage(nullptr);

			needsRender = 2; // 2 buffers
		}
		else
		{
			saveImagesAmnt = 0;
		}

		glfwPollEvents();

		if (needsRender > 0)
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if (selectedImageIndex != -1) editor.Render();

			RenderUI();

			// Decrement amnt of buffers to render, if user changes something, this will increase
			needsRender--;
		}

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			std::cout << "OpenGL error: " << error << std::endl;
		}

		glfwSwapBuffers(window);
	}
}

void App::OpenFolderContents(const std::string& folderPath) 
{
	std::vector<std::string> supportedFileTypes = { ".png", ".jpg", "jpeg", ".bmp" };

	std::filesystem::path path(folderPath);

	for (const auto& entry : std::filesystem::directory_iterator(path)) 
	{
		if (entry.is_regular_file()) 
		{
			std::string fileName = entry.path().filename().string();
			std::string extension = entry.path().extension().string();

			// Check if the file type is supported
			if (std::find(supportedFileTypes.begin(), supportedFileTypes.end(), extension) != supportedFileTypes.end()) 
			{
				std::filesystem::path filePath = entry.path();
				imagePathQueue.push_back(filePath.string());
			}
		}
	}

	wishImagesAmnt += imagePathQueue.size();
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

	int imageSettingsWindowWidth = 350;

	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::SetWindowSize(ImVec2(windowWidth - imageSettingsWindowWidth, windowHeight - 256 - 40));
	ImGui::SetWindowPos(ImVec2(0, 40));

	if (selectedImageIndex != -1)
	{
		int imageWidth = images[selectedImageIndex]->width;
		int imageHeight = images[selectedImageIndex]->height;
		float aspectRatioImage = (float)imageWidth / (float)imageHeight;

		int viewportWidth = windowWidth - imageSettingsWindowWidth;
		int viewportHeight = windowHeight - 256 - 60;
		float aspectRatioViewport = (float)viewportWidth / (float)(viewportHeight);

		int x, y;
		if (aspectRatioImage > aspectRatioViewport) {
			x = viewportWidth;
			y = viewportWidth / aspectRatioImage;
		}
		else {
			x = (viewportHeight) * aspectRatioImage;
			y = viewportHeight;
		}

		int offset = (viewportWidth - x) / 2;

		ImGui::SameLine(offset);
		ImGui::Image((ImTextureID)(intptr_t)editor.currentImage->textureOutID, ImVec2(x, y));
	}

	ImGui::End();

	ImGui::Begin("Image Settings", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::SetWindowSize(ImVec2(imageSettingsWindowWidth, windowHeight - 256 - 40));
	ImGui::SetWindowPos(ImVec2(windowWidth - imageSettingsWindowWidth, 40));
	editor.RenderUI();
	ImGui::End();

	ImGui::Begin("Gallery", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::SetWindowSize(ImVec2(windowWidth, 256));
	ImGui::SetWindowPos(ImVec2(0, windowHeight - 256));

	ImGui::Text("Gallery");

	ImGui::BeginChild("Images", ImVec2(0, 256 - 56), true, ImGuiWindowFlags_HorizontalScrollbar);

	int desiredHeight = 256 - 56 - 24 - ImGui::GetStyle().WindowPadding.y;

	for (int i = 0; i < images.size(); i++)
	{
		float aspectRatio = (float)images[i]->width / (float)images[i]->height;
		int desiredWidth = desiredHeight * aspectRatio;

		ImGui::Image((ImTextureID)(intptr_t)images[i]->textureOutID, ImVec2(desiredWidth, desiredHeight), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

		if (ImGui::IsItemClicked()) {
			selectedImageIndex = i;
			editor.SetImage(images[i]);
		}

		if (i != images.size() - 1) {
			ImGui::SameLine();
		}
	}

	ImGui::EndChild();

	if (images.size() != wishImagesAmnt)
	{
		float progressBarHeight = 24.0f;
		std::string label = "Loading images (" + std::to_string(images.size()) + "/" + std::to_string(wishImagesAmnt) + ")";

		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - progressBarHeight - ImGui::GetStyle().WindowPadding.y);

		ImGui::ProgressBar((float)images.size() / (float)wishImagesAmnt, ImVec2(windowWidth - 12, progressBarHeight), label.c_str());
	} 
	else if (imageSaveQueue.size() != 0)
	{
		float progressBarHeight = 24.0f;
		std::string label = "Saving images (" + std::to_string(imageSaveQueue.size()) + "/" + std::to_string(saveImagesAmnt) + ")";

		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - progressBarHeight - ImGui::GetStyle().WindowPadding.y);

		ImGui::ProgressBar((float)imageSaveQueue.size() / (float)saveImagesAmnt, ImVec2(windowWidth - 12, progressBarHeight), label.c_str());
	}
	else 
	{
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 15 - ImGui::GetStyle().WindowPadding.y);

		ImGui::Text("Loaded images: %i", images.size());
	}

	ImGui::End();

	if (fileWindowOpen) {
		ImGui::Begin("File window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::SetWindowSize(ImVec2(0, 0));
		ImGui::SetWindowPos(ImVec2(4, 32));
		if (ImGui::Selectable("Open folder"))
		{
			fileWindowOpen = false;
			IGFD::FileDialogConfig config;
			config.path = ".";
			ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", " Choose a File", ".*,.png,.gif,.jpg,.jpeg,.bmp", config);
		}

		if (selectedImageIndex >= 0 && selectedImageIndex < images.size()) 
		{
			if (ImGui::Selectable("Save image")) 
			{
				fileWindowOpen = false;
				wishImagesAmnt--;

				imageSaveQueue.push_back(images[selectedImageIndex]);

				images.erase(images.begin() + selectedImageIndex);

				selectedImageIndex = -1;
			}
		}

		if (selectedImageIndex >= 0 && selectedImageIndex < images.size()) 
		{
			if (ImGui::Selectable("Save all images"))
			{
				fileWindowOpen = false;
				wishImagesAmnt = 0;
				
				saveImagesAmnt = images.size();
				imageSaveQueue = images;
				images.clear();
				selectedImageIndex = -1;

				std::cout << imageSaveQueue.size() << std::endl;
			}
		}
		
		if (ImGui::Selectable("Exit"))
		{
			glfwSetWindowShouldClose(window, true);
		}

		ImGui::End();
	}

	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePathName = ImGuiFileDialog::Instance()->GetCurrentPath();
			std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
		}
		ImGuiFileDialog::Instance()->Close();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

App::~App() 
{
	for (int i = 0; i < images.size(); i++)
	{
		glDeleteTextures(1, &images[i]->textureInID);
		glDeleteTextures(1, &images[i]->textureOutID);
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
}