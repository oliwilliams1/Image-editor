# Image editor
A simple hardware-accelerated image editor with multiple post processing techniques and masked-based editing.

# Previews
![hero](readme-res/hero.png)
### Supports custom logo on 360 images
![360](readme-res/360.png)
### Ground view of unwrapped 360 image
![hero](readme-res/360-ground.png)

# Building
```bash
git clone --recursive https://github.com/oliwilliams1/Image-editor
cd Image-editor
mkdir build
cd build
cmake ..
```
This will generate build files that you can build with your own build chain (msvc, make, Xcode...)