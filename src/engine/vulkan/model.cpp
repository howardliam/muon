#include "engine/vulkan/model.hpp"

#include "assimp/Importer.hpp"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "utils.hpp"

#include <spdlog/spdlog.h>

namespace muon {

    /* Vertex */
    std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> binding_descriptions(1);

        binding_descriptions[0].binding = 0;
        binding_descriptions[0].stride = sizeof(Vertex);
        binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding_descriptions;
    }

    std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};

        uint32_t location = 0;
        attribute_descriptions.push_back({
            location++,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            offsetof(Vertex, position)
        });
        attribute_descriptions.push_back({
            location++,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            offsetof(Vertex, colour)
        });
        attribute_descriptions.push_back({
            location++,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            offsetof(Vertex, normal)
        });
        attribute_descriptions.push_back({
            location++,
            0,
            VK_FORMAT_R32G32_SFLOAT,
            offsetof(Vertex, tex_coord)
        });

        return attribute_descriptions;
    }

    bool Model::Vertex::operator==(const Vertex &other) const {
        return position == other.position
            && colour == other.colour
            && normal == other.normal
            && tex_coord == other.tex_coord;
    }

    /* Builder */
    void Model::Builder::loadModel(const std::string &path) {
        Assimp::Importer importer;

        auto flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_OptimizeMeshes;
        const aiScene *scene = importer.ReadFile(path, flags);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            spdlog::error("Error: {}", importer.GetErrorString());
            exit(exitcode::FAILURE);
        }

        if (scene->mNumMeshes > 0) {
            aiMesh *mesh = scene->mMeshes[0];

            for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
                Vertex vertex{};

                vertex.position.x = mesh->mVertices[i].x;
                vertex.position.y = mesh->mVertices[i].y;
                vertex.position.z = mesh->mVertices[i].z;

                if (mesh->HasNormals()) {
                    vertex.normal.x = mesh->mNormals[i].x;
                    vertex.normal.y = mesh->mNormals[i].y;
                    vertex.normal.z = mesh->mNormals[i].z;
                }

                if (mesh->HasTextureCoords(0)) {
                    vertex.tex_coord.x = mesh->mTextureCoords[0][i].x;
                    vertex.tex_coord.y = mesh->mTextureCoords[0][i].y;
                } else {
                    vertex.tex_coord.x = 0.0f;
                    vertex.tex_coord.y = 0.0f;
                }

                vertex.colour.x = 1.0f;
                vertex.colour.y = 1.0f;
                vertex.colour.z = 1.0f;

                vertices.push_back(vertex);
            }

            for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
                aiFace face = mesh->mFaces[i];
                for (uint32_t j = 0; j < face.mNumIndices; j++) {
                    indices.push_back(face.mIndices[j]);
                }
            }
        }
    }

    /* Model */
    Model::Model(Device &device, const Builder &builder) : device{device} {
        createVertexBuffer(builder.vertices);
        createIndexBuffer(builder.indices);
    }

    Model::~Model() = default;

    void Model::bind(VkCommandBuffer command_buffer) {
        const VkBuffer buffers[] = {vertex_buffer->getBuffer()};
        constexpr VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

        if (has_index_buffer) {
            vkCmdBindIndexBuffer(command_buffer, index_buffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void Model::draw(VkCommandBuffer command_buffer) {
        if (has_index_buffer) {
            vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);
        } else {
            vkCmdDraw(command_buffer, vertex_count, 1, 0, 0);
        }
    }

    void Model::createVertexBuffer(const std::vector<Vertex> &vertices) {
        vertex_count = static_cast<uint32_t>(vertices.size());

        uint32_t vertex_size = sizeof(vertices[0]);
        VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count;

        Buffer staging_buffer{
            device,
            vertex_size,
            vertex_count,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        staging_buffer.map();
        staging_buffer.writeToBuffer((void*)vertices.data());

        vertex_buffer = std::make_unique<Buffer>(
            device,
            vertex_size,
            vertex_count,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        device.copyBuffer(staging_buffer.getBuffer(), vertex_buffer->getBuffer(), buffer_size);
    }

    void Model::createIndexBuffer(const std::vector<uint32_t> &indices) {
        index_count = static_cast<uint32_t>(indices.size());
        has_index_buffer = index_count > 0;

        if (!has_index_buffer) {
            return;
        }

        uint32_t index_size = sizeof(indices[0]);
        VkDeviceSize buffer_size = sizeof(indices[0]) * index_count;

        Buffer staging_buffer{
            device,
            index_size,
            index_count,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        staging_buffer.map();
        staging_buffer.writeToBuffer((void *)indices.data());

        index_buffer = std::make_unique<Buffer>(
            device,
            index_size,
            index_count,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        device.copyBuffer(staging_buffer.getBuffer(), index_buffer->getBuffer(), buffer_size);
    }

    std::unique_ptr<Model> Model::fromFile(Device &device, const std::string &path) {
        Model::Builder builder{};
        builder.loadModel(path);
        spdlog::trace("Vertex count: {}", builder.vertices.size());
        spdlog::trace("Index count: {}", builder.indices.size());
        return std::make_unique<Model>(device, builder);
    }

}
