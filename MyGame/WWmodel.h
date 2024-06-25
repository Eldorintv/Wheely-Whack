#ifndef WWMODEL_H
#define WWMODEL_H

#include "WWhelpers.h"
#include "WWboundingbox.h"

extern VkRenderPass renderPass;
extern VkDescriptorSetLayout descriptorSetLayout;

class Model : VulkanHelper {
public:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;

    uint32_t textureIndex;
    glm::mat4 modelMatrix;

    BoundingBox boundingBox;

    const char* vertexShaderPath = "../MyGame/media/shaders/vertBP.spv";
    const char* fragmentShaderPath = "../MyGame/media/shaders/fragBP.spv";

    static Model Load(const char* path, uint32_t textureIndex);

    static Model LoadSkybox(const char* path, uint32_t textureIndex);

    void Destroy();

    void Render(VkCommandBuffer& commandBuffer);

    void translateModelMatrix(glm::vec3 v);

    void rotateModelMatrix(float angle, glm::vec3 axis);

private:

    std::vector<glm::vec3> transformedVertices(const std::vector<Vertex> vertices, const glm::mat4& transform);

    void loadModel(const char* path);

    void createVertexBuffer();

    void createIndexBuffer();

    void createGraphicsPipeline(bool isNormalModel = true);

    static std::vector<char> readFile(const std::string& filename);

    VkShaderModule createShaderModule(const std::vector<char>& code);

};

#endif

