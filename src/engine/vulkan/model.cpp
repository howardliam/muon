#include "engine/vulkan/model.hpp"

#include <spdlog/spdlog.h>

#include "assimp/Importer.hpp"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "utils/exitcode.hpp"

namespace muon {

    /* Vertex */
    std::vector<vk::VertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
        std::vector<vk::VertexInputBindingDescription> binding_descriptions(1);

        binding_descriptions[0].binding = 0;
        binding_descriptions[0].stride = sizeof(Vertex);
        binding_descriptions[0].inputRate = vk::VertexInputRate::eVertex;

        return binding_descriptions;
    }

    std::vector<vk::VertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
        std::vector<vk::VertexInputAttributeDescription> attribute_descriptions{};

        uint32_t location = 0;
        attribute_descriptions.push_back({
            location++,
            0,
            vk::Format::eR32G32B32Sfloat,
            offsetof(Vertex, position)
        });
        attribute_descriptions.push_back({
            location++,
            0,
            vk::Format::eR32G32B32Sfloat,
            offsetof(Vertex, colour)
        });
        attribute_descriptions.push_back({
            location++,
            0,
            vk::Format::eR32G32B32Sfloat,
            offsetof(Vertex, normal)
        });
        attribute_descriptions.push_back({
            location++,
            0,
            vk::Format::eR32G32Sfloat,
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
        // auto flags = aiProcess_Triangulate | aiProcess_GenNormals;
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

    void Model::bind(vk::CommandBuffer command_buffer) {
        const vk::Buffer buffers[] = {vertex_buffer->getBuffer()};
        constexpr vk::DeviceSize offsets[] = {0};

        command_buffer.bindVertexBuffers(0, buffers, offsets);

        if (has_index_buffer) {
            command_buffer.bindIndexBuffer(index_buffer->getBuffer(), 0, vk::IndexType::eUint32);
        }
    }

    void Model::draw(vk::CommandBuffer command_buffer) {
        if (has_index_buffer) {
            command_buffer.drawIndexed(index_count, 1, 0, 0, 0);
        } else {
            command_buffer.draw(vertex_count, 1, 0, 0);
        }
    }

    void Model::createVertexBuffer(const std::vector<Vertex> &vertices) {
        vertex_count = static_cast<uint32_t>(vertices.size());

        uint32_t vertex_size = sizeof(vertices[0]);
        vk::DeviceSize buffer_size = sizeof(vertices[0]) * vertex_count;

        Buffer staging_buffer{
            device,
            vertex_size,
            vertex_count,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        };

        staging_buffer.map();
        staging_buffer.writeToBuffer((void*)vertices.data());

        vertex_buffer = std::make_unique<Buffer>(
            device,
            vertex_size,
            vertex_count,
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vk::MemoryPropertyFlagBits::eDeviceLocal
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
        vk::DeviceSize buffer_size = sizeof(indices[0]) * index_count;

        Buffer staging_buffer{
            device,
            index_size,
            index_count,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        };

        staging_buffer.map();
        staging_buffer.writeToBuffer((void *)indices.data());

        index_buffer = std::make_unique<Buffer>(
            device,
            index_size,
            index_count,
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vk::MemoryPropertyFlagBits::eDeviceLocal
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
