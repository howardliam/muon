#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 tex_coord;

layout(location = 0) out vec3 out_colour;
layout(location = 1) out vec2 out_tex_coord;

layout(set = 0, binding = 0) uniform Ubo {
    mat4 projection;
    mat4 view;
} ubo;

layout(push_constant) uniform Push {
    mat4 model;
} push;

void main() {
    gl_Position = ubo.projection * ubo.view * push.model * vec4(position, 1.0);
    out_colour = colour;
    out_tex_coord = tex_coord;
}
