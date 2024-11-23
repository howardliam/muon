# Muon Engine
An amateur game engine for learning Vulkan among other technologies.

## Todo
### Graphics (Vulkan)
- [x] Initial Vulkan setup
- [x] Model loading*
- [x] Texture loading
- [ ] Good base render system as an example
- [x] Camera
- [ ] Text
- [ ] Multithreading
- [ ] Post processing
### Audio
- [ ] Audio (OpenAL)
### Game
- [ ] User input handling
- [ ] ECS implementation
- [ ] C# scripting with mono

\* Model loading only without indices.

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
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)
- [entt](https://github.com/skypjack/entt)
