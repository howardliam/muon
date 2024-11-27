# Muon Engine
An amateur game engine for learning Vulkan among other technologies.

## Todo
### Graphics (Vulkan)
- [x] Initial Vulkan setup
- [x] Model loading
- [x] Texture loading
- [ ] Good base render system as an example
- [x] Camera
- [ ] Text
- [ ] Multithreading
- [ ] Post processing
- [x] Switch from tinyobjloader to Assimp
- [x] Improve texture loading
### Audio
- [ ] Audio (OpenAL)
### Networking
- [ ] TCP support
- [ ] UDP support
### Game
- [ ] User input handling
- [ ] ECS implementation
- [ ] C# scripting with mono
### Assets
- [ ] Asset packing

## Build instructions
```
git clone https://github.com/howardliam/muon
cd muon
bash setup.sh
bash run.sh
```

## Attributions and Requirements
- [Vulkan](https://www.vulkan.org/)
- [SDL3](https://www.libsdl.org/index.php)
- [spdlog](https://github.com/gabime/spdlog)
- [glm](https://github.com/g-truc/glm)
- [STB](https://github.com/nothings/stb) *
- [entt](https://github.com/skypjack/entt) *
- [cpptoml](https://github.com/skystrife/cpptoml) *
- [assimp](https://github.com/assimp/assimp) *
- [msdf-atlas-gen](https://github.com/Chlumsky/msdf-atlas-gen) *

\* provided as a submodule

## Additional notes
### Conventions
- Classes/structs are PascalCase
- Functions/methods are camelCase
- Variables and object members are snake_case
