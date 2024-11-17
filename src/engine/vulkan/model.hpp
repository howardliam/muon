#pragma once

#include <vector>

#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "device.hpp"
#include "buffer.hpp"

class Model {
public:
    struct Vertex {
        glm::vec3 position{};
        glm::vec3 colour{};
        glm::vec3 normal{};
        glm::vec2 tex_coord{};

        static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
        static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();

        bool operator==(const Vertex &other) const;
    };

    struct Builder {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};

        void load_model(const std::string &path);
    };

    Model(Device &device, const Builder &builder);
    ~Model();

    Model(const Model &) = delete;
    Model& operator=(const Model &) = delete;

    void bind(VkCommandBuffer command_buffer);
    void draw(VkCommandBuffer command_buffer);

private:
    Device &device;

    std::unique_ptr<Buffer> vertex_buffer;
    uint32_t vertex_count;

    bool has_index_buffer = false;
    std::unique_ptr<Buffer> index_buffer;
    uint32_t index_count;

    void create_vertex_buffer(const std::vector<Vertex> &vertices);
    void create_index_buffer(const std::vector<uint32_t> &indices);
};

std::unique_ptr<Model> create_model_from_file(Device &device, const std::string &path);
