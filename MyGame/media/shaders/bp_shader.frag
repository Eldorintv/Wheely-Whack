#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

const float lightPower = 25.0;
const vec3 ambientColor = vec3(0.1, 0.1, 0.1);
const vec3 diffuseColor = vec3(0.2, 0.2, 0.2);
const vec3 specColor = vec3(1.0, 1.0, 1.0);
const float shininess = 16.0;
const vec3 lightPos = vec3(-5.0, 8.0, 1.0);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);

void main() {
    vec3 cameraPos = inverse(ubo.view)[3].xyz;

    // from wikipedia 
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = lightPos - fragWorldPos;
    float distance = length(lightDir);
    distance = distance * distance;
    lightDir = normalize(lightDir);

    float lambertian = max(dot(lightDir, fragNormal), 0.0);
    float specular = 0.0;

    if (lambertian > 0.0) {
        // this is different from wikipedia, I need the cameraPos
        vec3 viewDir = normalize(cameraPos - fragWorldPos);

        // blinn phong
        vec3 halfDir = normalize(lightDir + viewDir);
        float specAngle = max(dot(halfDir, fragNormal), 0.0);
        specular = pow(specAngle, shininess);
    }
    vec3 colorLinear = texture(texSampler, fragTexCoord).rgb +
                     diffuseColor * lambertian * lightColor * lightPower / distance +
                     specColor * specular * lightColor * lightPower / distance;

    outColor = vec4(colorLinear, 1.0);
}