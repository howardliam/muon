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
bash setup.sh
bash run.sh
```

## Attributions
- [STB](https://github.com/nothings/stb)
- [entt](https://github.com/skypjack/entt)

## Additional notes
### Conventions
- Classes/structs are PascalCase
- Functions/methods are camelCase
- Variables and object members are snake_case
