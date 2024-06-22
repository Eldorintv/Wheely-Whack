#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;


layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragTexCoord;

void main()
{
    vec4 pos = ubo.proj * ubo.view * vec4(inPosition, 1.0);
    gl_Position = pos.xyww; // z is not here on purpose, no depth testing
    //fragColor = inColor;
    fragTexCoord = vec3(inTexCoord, 1.0); // not correct, maybe fix later
}