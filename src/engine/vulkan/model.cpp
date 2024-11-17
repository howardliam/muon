#include "model.hpp"

#include <unordered_map>

#include <spdlog/spdlog.h>

#include "../../tinyobjloader/tiny_obj_loader.h"

/* Vertex */
std::vector<VkVertexInputBindingDescription> Model::Vertex::get_binding_descriptions() {
    std::vector<VkVertexInputBindingDescription> binding_descriptions(1);

    binding_descriptions[0].binding = 0;
    binding_descriptions[0].stride = sizeof(Vertex);
    binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return binding_descriptions;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::get_attribute_descriptions() {
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
void Model::Builder::load_model(const std::string &path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        spdlog::error("{}", warn + err);
        exit(exitcode::FAILURE);
    }

    vertices.clear();
    indices.clear();

    /* Fix this fucking shit */
    // std::unordered_map<Vertex, uint32_t> unique_vertices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            if (index.vertex_index >= 0) {
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };

                vertex.colour = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2],
                };
            }

            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }
            if (index.texcoord_index >= 0) {
                vertex.tex_coord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1],
                };
            }

            // if (unique_vertices.find(vertex) == unique_vertices.end()) {
            //     unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
            //     vertices.push_back(vertex);
            // }
            // indices.push_back(unique_vertices[vertex]);

            vertices.push_back(vertex);
        }
    }
}

/* Model */
Model::Model(Device &device, const Builder &builder) : device{device} {
    create_vertex_buffer(builder.vertices);
    create_index_buffer(builder.indices);
}

Model::~Model() = default;

void Model::bind(VkCommandBuffer command_buffer) {
    const VkBuffer buffers[] = {vertex_buffer->get_buffer()};
    constexpr VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

    if (has_index_buffer) {
        vkCmdBindIndexBuffer(command_buffer, index_buffer->get_buffer(), 0, VK_INDEX_TYPE_UINT32);
    }
}

void Model::draw(VkCommandBuffer command_buffer) {
    if (has_index_buffer) {
        vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);
    } else {
        vkCmdDraw(command_buffer, vertex_count, 1, 0, 0);
    }
}

void Model::create_vertex_buffer(const std::vector<Vertex> &vertices) {
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
    staging_buffer.write_to_buffer((void*)vertices.data());

    vertex_buffer = std::make_unique<Buffer>(
        device,
        vertex_size,
        vertex_count,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    device.copy_buffer(staging_buffer.get_buffer(), vertex_buffer->get_buffer(), buffer_size);
}

void Model::create_index_buffer(const std::vector<uint32_t> &indices) {
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
    staging_buffer.write_to_buffer((void*)indices.data());

    index_buffer = std::make_unique<Buffer>(
        device,
        index_size,
        index_count,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    device.copy_buffer(staging_buffer.get_buffer(), index_buffer->get_buffer(), buffer_size);
}

std::unique_ptr<Model> create_model_from_file(Device &device, const std::string &path) {
    Model::Builder builder{};
    builder.load_model(path);
    spdlog::debug("Vertex count {}", builder.vertices.size());
    return std::make_unique<Model>(device, builder);
}
