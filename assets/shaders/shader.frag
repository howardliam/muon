#version 450

layout(location = 0) in vec3 colour;
layout(location = 1) in vec2 tex_coord;

layout(location = 0) out vec4 frag_colour;

layout(set = 0, binding = 0) uniform Ubo {
    mat4 projection;
    mat4 view;
} ubo;

layout(push_constant) uniform Push {
    mat4 model;
} push;

void main() {
    // frag_colour = texture(tex, tex_coord);
    frag_colour = vec4(colour, 0.0);
}
