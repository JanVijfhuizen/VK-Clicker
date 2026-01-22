#version 450

layout(set = 0, binding = 0) uniform ColorUBO {
    vec3 color;
} ubo;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(ubo.color, 1.0);
}