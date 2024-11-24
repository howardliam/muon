#version 450

layout(location = 0) in vec3 colour;
layout(location = 1) in vec2 tex_coord;

layout(location = 0) out vec4 frag_colour;

layout(set = 0, binding = 0) uniform Ubo {
    mat4 projection;
    mat4 view;
} ubo;

layout(binding = 1) uniform sampler2D msdf;

layout(push_constant) uniform Push {
    mat4 model;
} push;

const float pxRange = 1.0;
const vec3 bg_colour = vec3(0.0, 0.0, 0.0);

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange() {
    vec2 unit_range = vec2(pxRange) / vec2(textureSize(msdf, 0));
    vec2 screenTexSize = vec2(1.0) / fwidth(tex_coord);
    return max(0.5 * dot(unit_range, screenTexSize), 1.0);
}

void main() {
    vec3 msd = texture(msdf, tex_coord).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange() * (sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    frag_colour = vec4(mix(bg_colour, colour, opacity), 1.0);
}
