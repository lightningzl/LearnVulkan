#version 450

vec2 positions[3] = vec2[]
(
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

layout(location = 0) in vec2 Position;

layout(set = 0, binding = 0) uniform UniformBuffer
{
    mat4 project;
    mat4 view;
    mat4 model;
} ubo;

void main()
{
    gl_Position = ubo.project * ubo.view * ubo.model * vec4(Position, 0.0, 1.0);
    //gl_Position = vec4(Position, 0.0, 1.0);
}
