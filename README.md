# Muon Engine
An amateur game engine for learning Vulkan among other technologies.

## Todo
### Graphics (Vulkan)
- [x] Initial Vulkan setup
- [x] Model loading
  - [x] Switch from tinyobjloader to Assimp
- [x] Texture loading
  - [x] Improve texture loading
  - [ ] Replace stb_image
- [ ] Camera
  - [x] Perspective projections
  - [ ] Orthographic projections
  - [x] Look at
- [ ] Text
  - [x] MSDF text printing with transparency
  - [ ] Text rendering with orthographic projection
  - [ ] Text rendering pipeline
- [ ] Render modules
  - [ ] Figure out some system for this
- [ ] Render graph
  - [ ] Using Framegraph (maybe?)
- [ ] Multithreading
- [ ] Post processing
  - [ ] Offscreen framebuffer rendering
  - [ ] Pipeline chain for effects

### Audio
- [ ] Audio (OpenAL)
  - [ ] Ogg-Vorbis support
  - [ ] Audio subsystem (something like Paul's Soundsystem)

### Networking
- [ ] Networking
  - [ ] Pick a networking library
  - [ ] TCP support
  - [ ] UDP support

### Game
- [ ] User input
  - [ ] Keyboard handling
  - [ ] Mouse handling
  - [ ] Control scheme file
- [ ] ECS implementation
- [ ] C# scripting with Mono
- [ ] Saving game data
  - [ ] Object serialisation
  - [ ] Serialised adata compression
    - [ ] LZ4 support

### Assets
- [ ] Asset packing
- [ ] Asset store (storage not a shop)

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
- [tomlplusplus](https://github.com/marzer/tomlplusplus) *
- [assimp](https://github.com/assimp/assimp) *
- [msdf-atlas-gen](https://github.com/Chlumsky/msdf-atlas-gen) *

\* provided as a submodule

## Additional notes
### Conventions
- Classes/structs are PascalCase
- Functions/methods are camelCase
- Variables and object members are snake_case
- Abbreviations and initialisations count as a word and
  follow naming conventions as such, example:
  - `initializeSdl`, `loadGltfModel`
- Project uses American English for consistency
