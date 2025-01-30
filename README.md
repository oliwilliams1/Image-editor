# Image editor
A simple hardware-accelerated image editor with multiple post processing techniques and masked-based editing.

# Building

### Building for Windows via CMake & MSVC
To build this project for Windows using CMake and MSVC, paste these commands into your terminal to setup the .slm file
```bash
git clone https://github.com/oliwilliams1/Image-editor "Image editor" --recursive
cd "Image editor"
mkdir build
cd build
cmake ..
```
Once CMake configuration is comnplete, open ```SableImageEditor.sln``` locatied in the /build directory.
In the Solution explorer, right click on SableImageEditor and select "Set as startup project"
Select either debug or release then build the solution. This will compile the project and create the executable.