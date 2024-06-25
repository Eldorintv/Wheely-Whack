#ifndef WWTEXTURE_H
#define WWTEXTURE_H

#include "WWhelpers.h"

class Texture : VulkanHelper {
public:
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;


    static Texture Load(const char* path);

    static Texture LoadCubeMap(const char* front, const char* back, const char* up, const char* down, const char* right, const char* left);

    void Destroy();
private:
    void createTextureImage(const char* path);

    std::vector<stbi_uc*> loadCubeMapImages(std::vector<const char*>& paths, int& texWidth, int& texHeight, int& texChannels);

    void createTextureImageCubeMap(std::vector<const char*>& paths);

    void createTextureImageView(uint32_t layerCount = 1);

    void createTextureSampler(bool isCube = false);
};

#endif

