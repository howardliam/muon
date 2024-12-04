#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "engine/vulkan/device.hpp"
#include "engine/vulkan/buffer.hpp"

namespace muon {

    class Model {
    public:
        struct Vertex {
            glm::vec3 position{};
            glm::vec3 colour{};
            glm::vec3 normal{};
            glm::vec2 tex_coord{};

            static std::vector<vk::VertexInputBindingDescription> getBindingDescriptions();
            static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex &other) const;
        };

        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string &path);
        };

        Model(Device &device, const Builder &builder);
        ~Model();

        Model(const Model &) = delete;
        Model& operator=(const Model &) = delete;

        static std::unique_ptr<Model> fromFile(Device &device, const std::string &path);

        void bind(vk::CommandBuffer command_buffer);
        void draw(vk::CommandBuffer command_buffer);

    private:
        Device &device;

        std::unique_ptr<Buffer> vertex_buffer;
        uint32_t vertex_count;

        bool has_index_buffer = false;
        std::unique_ptr<Buffer> index_buffer;
        uint32_t index_count;

        void createVertexBuffer(const std::vector<Vertex> &vertices);
        void createIndexBuffer(const std::vector<uint32_t> &indices);
    };

}
