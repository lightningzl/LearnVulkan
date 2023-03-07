#version 450

vec2 positions[3] = vec2[]
(
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

layout(location = 0) in vec2 Position;
layout(location = 1) in vec2 inTexcoord;

layout(location = 0) out vec2 outTexcoord;

layout(set = 0, binding = 0) uniform UniformBuffer
{
    mat4 project;
    mat4 view;
} ubo;

layout(push_constant) uniform PushConstant
{
    mat4 model;
} pc;

void main()
{
    gl_Position = ubo.project * ubo.view * pc.model * vec4(Position, 0.0, 1.0);
    outTexcoord = inTexcoord;
    //gl_Position = vec4(Position, 0.0, 1.0);
}
