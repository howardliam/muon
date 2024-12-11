# Muon Engine
An amateur game engine for learning Vulkan among other technologies.

## Todo
### Global
- [x] Switch from using C integer types (int, short, byte, etc.)
to cstdint (int32_t, int16_t, int8_t, etc.) where applicable
- [ ] Add \[\[nodiscard\]\] to getters and other methods
### Graphics (Vulkan)
- [x] Initial Vulkan setup
- [x] Port to using full C++ headers
- [x] Model loading
  - [x] Switch from tinyobjloader to Assimp
  - [ ] Remove colour as vertex attribute and move it to UBO's/push constants where applicable
- [x] Texture loading
  - [x] Improve texture loading
  - [x] Replace stb_image
  - [ ] Add support for other formats than PNG
- [ ] Camera
  - [x] Perspective projections
  - [x] Orthographic projections
  - [x] View matrix (look at)
- [ ] Text
  - [x] MSDF text printing with transparency
  - [ ] Text rendering with orthographic projection
  - [ ] Text rendering pipeline
- [ ] Render modules
  - [ ] Figure out some system for this
- [ ] Render graph
  - [ ] Using [FrameGraph](https://github.com/skaarj1989/FrameGraph) (maybe?)
- [ ] Multithreading
- [ ] Post processing
  - [ ] Offscreen framebuffer rendering
  - [ ] Pipeline chain for effects
    - [ ] Anti aliasing
    - [ ] Ambient occlusion
    - [ ] Screen space reflections
- [ ] Descriptors
  - [ ] Descriptor sets
    - [ ] Global one for: camera, lights, other stuff
    - [ ] One for textures for models
    - [ ] One for other buffers like model transform data

### Audio
- [ ] Audio (OpenAL)
  - [ ] Ogg-Vorbis support
  - [ ] Audio subsystem (something like Paul's Code Soundsystem)

### Networking
- [ ] Networking
  - [ ] [GameNetworkingSockets](https://github.com/ValveSoftware/GameNetworkingSockets) ~~Pick a networking library~~
  - [ ] TCP support
  - [ ] UDP support

### Game
- [x] User input
  - [x] Keyboard handling
  - [x] Mouse handling
  - [ ] Control scheme file
- [ ] ECS implementation
- [ ] C# scripting with Mono
- [ ] Saving game data
  - [ ] Object serialisation
  - [ ] Serialised adata compression
    - [ ] LZ4 support

### Assets
- [ ] Asset packing
- [ ] Asset storage
  - [ ] Handling of loading assets separated from texture, model, sound classes, etc.
  - [ ] Asynchronous asset loading
  - [ ] Resource cache

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
- [entt](https://github.com/skypjack/entt) *
- [tomlplusplus](https://github.com/marzer/tomlplusplus) *
- [assimp](https://github.com/assimp/assimp) *
- [msdf-atlas-gen](https://github.com/Chlumsky/msdf-atlas-gen) *
- [stb_vorbis](https://github.com/nothings/stb) **

\* provided as a submodule
\** included in source tree

## Additional notes
### Conventions
- Classes/structs are PascalCase
- Functions/methods are camelCase
- Variables and object members are snake_case
- Abbreviations and initialisations count as a word and
  follow naming conventions as such, example:
  - `initializeSdl`, `loadGltfModel`
- Project uses American English for consistency
