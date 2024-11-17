# Vulkan SDL3 Starter Project
A starter project for game engine/games built on SDL3 using Vulkan for graphics API.

## Todo
- [x] Initial Vulkan setup
- [x] Model loading*
- [ ] Texture loading
- [ ] Game object wrapper
- [ ] Camera
- [ ] Good base render system as an example
- [ ] User input handling
- [ ] Audio (OpenAL)

\* Model loading only without indices.

## Requirements
- SDL3
- Vulkan
- glm
- spdlog

## Build instructions
```
git clone https://github.com/howardliam/vulkan-sdl3-starter
cd vulkan-sdl3-starter
mkdir build
cd build
cmake -G Ninja ..
cd ..
cmake --build build
./build/vulkan-sdl3-starter
```

## Additional thanks to
- [STB](https://github.com/nothings/stb)
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)
