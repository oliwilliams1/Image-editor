cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(SableImageEditor VERSION 1.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(src)

# Fetch dependencies
include(FetchContent)

# OpenGL
find_package(OpenGL REQUIRED)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
)
FetchContent_MakeAvailable(glm)

# GLFW (Static)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
)
FetchContent_MakeAvailable(glfw)

# GLEW
FetchContent_Declare(
    glew
    GIT_REPOSITORY https://github.com/nigels-com/glew.git
)
FetchContent_MakeAvailable(glew)

# ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
)
FetchContent_MakeAvailable(imgui)

# Native file dialog
FetchContent_Declare(
    nfd
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
)
FetchContent_MakeAvailable(nfd)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    ${OPENGL_LIBRARIES} 
    glfw
    ${GLEW_LIBRARIES}
    glm
    imgui
    nfd
)

# Set the output directory to the project root
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)