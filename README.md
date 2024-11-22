# Muon Engine
An amateur game engine for learning Vulkan among other technologies.

## Todo
- [x] Initial Vulkan setup
- [x] Model loading*
- [x] Texture loading
- [ ] Game object wrapper
- [ ] Camera
- [ ] Good base render system as an example
- [ ] User input handling
- [x] Audio (OpenAL)**
- [ ] Text

\* Model loading only without indices.
\** Rather simple implementation.

## Requirements
- SDL3
- Vulkan
- glm
- spdlog
- OpenAL

## Build instructions
```
git clone https://github.com/howardliam/muon
cd muon
mkdir build
cd build
cmake -G Ninja ..
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=YES .
cd ..
cmake --build build
./build/muon
```

## Attributions
- [STB](https://github.com/nothings/stb)
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)
- [entt](https://github.com/skypjack/entt)
