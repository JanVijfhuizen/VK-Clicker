#version 450

layout(location = 0) in vec2 inPos;

layout(push_constant) uniform PushConstants {
    mat4 transform;
} pc;

void main() {
    gl_Position = pc.transform * vec4(inPos, 0.0, 1.0);
}