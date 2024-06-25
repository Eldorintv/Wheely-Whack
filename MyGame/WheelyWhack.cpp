#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION

#include "WWInclude.h"
#include "WWencoder.h"


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

Camera camera;

bool wIsPressed = false;
bool sIsPressed = false;;
bool aIsPressed = false;;
bool dIsPressed = false;;

VkCommandPool commandPool;
VkDevice device;
VkQueue graphicsQueue;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

VkRenderPass renderPass;
VkDescriptorSetLayout descriptorSetLayout;

const int MAX_FRAMES_IN_FLIGHT = 2;

std::chrono::milliseconds FPS_INTERVAL(30);
auto lastExecuted = std::chrono::system_clock::now();

// encoder and gpu queue
std::queue<uint8_t*> encQueue;
std::mutex encQueueMutex;
std::condition_variable encQueueCondition;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


class HelloTriangleApplication : VulkanHelper {
public:
    void run() {
        initWindow();
        initVulkan();
        std::thread encodingThread(&HelloTriangleApplication::encoderQueueAndUDPSend, this);
        mainLoop();
        encodeOn = false;
        encodingThread.join();
        cleanup();
    }

private:
    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    bool framebufferResized = false;

    std::vector<std::string> texturePathArray;
    std::vector<std::string> objectPathArray;


    std::vector<Texture> textures;

    std::vector<Model> models;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkDeviceSize bufferSize;

    VkBufferImageCopy region = {};

    VkCommandBuffer commandBufferImageCopy;

    std::atomic<bool> encodeOn = true;


    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Wheely Whack - Server", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetKeyCallback(window, key_callback);
        glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);
        glfwSetCursorPosCallback(window, cursorPosition_callback);
        glfwSetMouseButtonCallback(window, mouseButton_callback);

    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createDescriptorSetLayout();
        createCommandPool();
        createDepthResources();
        createFramebuffers();

        // not working as expected yet:
        // TEXTURES
        textures.push_back(Texture::LoadCubeMap("../MyGame/media/textures/sky/bluecloud_ft.jpg",
            "../MyGame/media/textures/sky/bluecloud_bk.jpg",
            "../MyGame/media/textures/sky/bluecloud_up.jpg",
            "../MyGame/media/textures/sky/bluecloud_dn.jpg",
            "../MyGame/media/textures/sky/bluecloud_rt.jpg",
            "../MyGame/media/textures/sky/bluecloud_lf.jpg"));
        textures.push_back(Texture::Load("../MyGame/media/textures/road.jpg")); //1
        textures.push_back(Texture::Load("../MyGame/media/textures/rainbowMarmor.jpg"));//2
        textures.push_back(Texture::Load("../MyGame/media/textures/paper.jpg"));//3
        textures.push_back(Texture::Load("../MyGame/media/textures/water.png"));//4
        textures.push_back(Texture::Load("../MyGame/media/textures/rock.jpg"));//5
        textures.push_back(Texture::Load("../MyGame/media/textures/yellow_star.png"));//6
        textures.push_back(Texture::Load("../MyGame/media/textures/cone.png"));//7

        // MODELS
        models.push_back(Model::LoadSkybox("../MyGame/media/models/skybox_cube.obj", 0)); // 0
        models.push_back(Model::Load("../MyGame/media/models/line_road/road.obj", 1)); // 1
        models.push_back(Model::Load("../MyGame/media/models/line_road/water_left.obj", 4)); // 2
        models.push_back(Model::Load("../MyGame/media/models/line_road/water_right.obj", 4)); // 3
        models.push_back(Model::Load("../MyGame/media/models/line_road/left_iron_fence.obj", 2)); //4
        models.push_back(Model::Load("../MyGame/media/models/line_road/right_iron_fence.obj", 2));//5
        models.push_back(Model::Load("../MyGame/media/models/line_road/road2.obj", 1));//6
        models.push_back(Model::Load("../MyGame/media/models/line_road/left_fence2.obj", 2));//7
        models.push_back(Model::Load("../MyGame/media/models/line_road/right_fence2.obj", 2));//8
        models.push_back(Model::Load("../MyGame/media/models/line_road/road3.obj", 1));//9 rainbow special level
        models.push_back(Model::Load("../MyGame/media/models/line_road/left_fence3.obj", 2));//10
        models.push_back(Model::Load("../MyGame/media/models/line_road/right_fence3.obj", 2));//11
        models.push_back(Model::Load("../MyGame/media/models/line_road/road4.obj", 2));//12
        models.push_back(Model::Load("../MyGame/media/models/line_road/left_fence4.obj", 2));//13
        models.push_back(Model::Load("../MyGame/media/models/line_road/right_fence4.obj", 2));//14

        // the rock(s) xD
        models.push_back(Model::Load("../MyGame/media/models/cone.obj", 7));//15
        models.push_back(Model::Load("../MyGame/media/models/rock.obj", 5));//16
        models.push_back(Model::Load("../MyGame/media/models/cone.obj", 7));//17
        models.push_back(Model::Load("../MyGame/media/models/rock.obj", 5));//18
        models.push_back(Model::Load("../MyGame/media/models/cone.obj", 7));//19

        models.push_back(Model::Load("../MyGame/media/models/rock.obj", 5));//20
        models.push_back(Model::Load("../MyGame/media/models/cone.obj", 7));//21
        models.push_back(Model::Load("../MyGame/media/models/rock.obj", 5));//22
        models.push_back(Model::Load("../MyGame/media/models/cone.obj", 7));//23
        models.push_back(Model::Load("../MyGame/media/models/rock.obj", 5));//24

        models.push_back(Model::Load("../MyGame/media/models/cone.obj", 7));//25
        models.push_back(Model::Load("../MyGame/media/models/rock.obj", 5));//26
        models.push_back(Model::Load("../MyGame/media/models/cone.obj", 7));//27
        models.push_back(Model::Load("../MyGame/media/models/rock.obj", 5));//28
        models.push_back(Model::Load("../MyGame/media/models/cone.obj", 7));//29

        models.push_back(Model::Load("../MyGame/media/models/finish_line.obj", 3)); // 30 finish line
        // water fixes
        models.push_back(Model::Load("../MyGame/media/models/line_road/water_left2.obj", 4)); // 31
        models.push_back(Model::Load("../MyGame/media/models/line_road/water_right2.obj", 4)); // 32
        models.push_back(Model::Load("../MyGame/media/models/line_road/water_left3.obj", 4)); // 33
        models.push_back(Model::Load("../MyGame/media/models/line_road/water_right3.obj", 4)); // 34
        models.push_back(Model::Load("../MyGame/media/models/line_road/water_left4.obj", 4)); // 35
        models.push_back(Model::Load("../MyGame/media/models/line_road/water_right4.obj", 4)); // 36

        // star
        models.push_back(Model::Load("../MyGame/media/models/star.obj", 6)); // 37
        models[37].translateModelMatrix(glm::vec3(1170.0f, 0.0f, 0.0));

        resetRockPositions(true);

        createUniformBuffers();
        createDescriptorPool();

        createDescriptorSets();
        createCommandBuffers();
        createSyncObjects();

        createStagingBuffer();

    }

    double lastFrameTime = 0.0;
    double deltaTime = 0.0;

    std::vector<size_t> hittableObjects{
        /*fences*/
        4, 5, 7, 8, 10, 11, 13, 14, 
        /*rocks*/
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29
    };

    std::vector<size_t> finishLine{ 30 };

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            double currentFrameTime = glfwGetTime();
            deltaTime = currentFrameTime - lastFrameTime;
            lastFrameTime = currentFrameTime;
            glfwPollEvents();
            translateViewMatrix(deltaTime);

            checkForGameMechanics();
            drawFrame();
        }

        vkDeviceWaitIdle(device);
    }

    void checkForGameMechanics() {
        // check if something is being hit
        checkForHit(hittableObjects);
        checkForHit(finishLine);

        // this means we have reached the last rock, and all rocks are gonna be replaced
        if (camera.getPosition().x > models[29].modelMatrix[3].x) {
            // this means we have reached the last rock, and all rocks are gonna be replaced
            resetRockPositions(false); // false means not to origin
        }

        // When x > 1150 reset to 750
        //if (camera.getPosition().x > 1150) {
        //    camera.resetViewMatrix(glm::vec3(800.0f, 0.5f, 0.0f));
        //    camera.maxSpeed += 0.1f;
        //}
    }

    // not working at all
    void checkForHit(std::vector<size_t>& hitables) {
        for (const auto& object : hitables) {

            // finish line reached
            if (models[30].boundingBox.isInside(camera.getPosition())) {
                camera.resetViewMatrix();
                camera.carStarted = false;
                camera.velocity = 0.005f;
                resetRockPositions();

                // TO DO: option to restart
                return;
            }

            // if camera hits one of the objects in hittableObjects this evaluates to true
            if (models[object].boundingBox.isInside(camera.getPosition())) {
                camera.resetViewMatrix();
                camera.carStarted = false;
                camera.velocity = 0.005f;
                resetRockPositions();

                return;
            }
        }
        return;
    }

    void cleanupSwapChain() {
        vkDestroyImageView(device, depthImageView, nullptr);
        vkDestroyImage(device, depthImage, nullptr);
        vkFreeMemory(device, depthImageMemory, nullptr);

        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    void cleanup() {
        cleanupSwapChain();

        vkDestroyRenderPass(device, renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(device, uniformBuffers[i], nullptr);
            vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

        for (Texture texture : textures) {
            texture.Destroy();
        }

        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

        for (Model model : models) {
            model.Destroy();
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        // stagingBuffer for copying image out of gpu
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);

        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);


        glfwDestroyWindow(window);

        glfwTerminate();

        
    }

    void recreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device);

        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        createDepthResources();
        createFramebuffers();
    }

    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    void createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());

        for (uint32_t i = 0; i < swapChainImages.size(); i++) {
            swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    void createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }



    void createFramebuffers() {
        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = {
                swapChainImageViews[i],
                depthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics command pool!");
        }
    }

    void createDepthResources() {
        VkFormat depthFormat = findDepthFormat();

        createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat findDepthFormat() {
        return findSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    bool hasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void createUniformBuffers() {
        // allocating buffer size for all models once
        VkDeviceSize bufferSize = sizeof(UniformBufferObject) * models.size();

        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

            vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
        }
    }

    void createDescriptorPool() {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * models.size());

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * models.size());

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void createDescriptorSets() {
        size_t numModels = models.size();
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT * numModels, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * numModels);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT * numModels);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            for (size_t modelIndex = 0; modelIndex < numModels; modelIndex++) {
                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = uniformBuffers[i];
                bufferInfo.offset = sizeof(UniformBufferObject) * modelIndex;
                bufferInfo.range = sizeof(UniformBufferObject);

                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = textures[models[modelIndex].textureIndex].textureImageView;
                imageInfo.sampler = textures[models[modelIndex].textureIndex].textureSampler;

                std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

                descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[0].dstSet = descriptorSets[i * numModels + modelIndex];
                descriptorWrites[0].dstBinding = 0;
                descriptorWrites[0].dstArrayElement = 0;
                descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrites[0].descriptorCount = 1;
                descriptorWrites[0].pBufferInfo = &bufferInfo;

                descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[1].dstSet = descriptorSets[i * numModels + modelIndex];
                descriptorWrites[1].dstBinding = 1;
                descriptorWrites[1].dstArrayElement = 0;
                descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[1].descriptorCount = 1;
                descriptorWrites[1].pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
            }
        }
    }

    void createCommandBuffers() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        uint32_t descriptorSetIndex{ 0 };

        for (uint32_t i{ 0 }; i < models.size(); ++i) {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, models[i].graphicsPipeline);
            descriptorSetIndex = currentFrame * static_cast<uint32_t>(models.size()) + i;
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, models[i].pipelineLayout, 0, 1, &descriptorSets[descriptorSetIndex], 0, nullptr);
            models[i].Render(commandBuffer);
        }

        vkCmdEndRenderPass(commandBuffer);


        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        for (size_t i = 0; i < models.size(); ++i) {
            //models[i].updateModelMatrix(time);

            UniformBufferObject ubo{};
            ubo.model = models[i].modelMatrix;
            ubo.view = camera.getViewMatrix();
            ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 100.0f);
            ubo.proj[1][1] *= -1;

            void* dest = static_cast<char*>(uniformBuffersMapped[currentImage]) + i * sizeof(UniformBufferObject);
            memcpy(dest, &ubo, sizeof(ubo));
        }
    }

    void createStagingBuffer() {
        bufferSize = static_cast<VkDeviceSize>(swapChainExtent.width) * swapChainExtent.height * 4;

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { swapChainExtent.width, swapChainExtent.height, 1 };
    }

    void drawFrame() {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        updateUniformBuffer(currentFrame);

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }


        // copy image to host and put into encoder queue magic here
        auto now = std::chrono::system_clock::now();
        if (now - lastExecuted >= FPS_INTERVAL && encodeOn) {
            lastExecuted = now;

            commandBufferImageCopy = beginSingleTimeCommands();
            transitionImageLayout(swapChainImages[imageIndex], VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

            vkCmdCopyImageToBuffer(commandBufferImageCopy, swapChainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);

            endSingleTimeCommands(commandBufferImageCopy);

            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
            
            uint8_t* mappedData = new uint8_t[bufferSize];
            std::memcpy(mappedData, data, bufferSize);

            {
                std::lock_guard<std::mutex> lock(encQueueMutex);
                encQueue.push(std::move(mappedData));

                if (encQueue.size() > 15) {
                    FPS_INTERVAL += std::chrono::milliseconds(2);
                }
                else if (encQueue.size() < 2 && FPS_INTERVAL > std::chrono::milliseconds(40)) {
                    FPS_INTERVAL -= std::chrono::milliseconds(5);
                }
            }
            encQueueCondition.notify_one();

            vkUnmapMemory(device, stagingBufferMemory);

        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    void encoderQueueAndUDPSend() {
        Encoder encoder;
        encoder.codecReady = encoder.setUpCodec(swapChainExtent.width, swapChainExtent.height);

        while (encodeOn) {
            uint8_t* frame;
            {
                std::unique_lock<std::mutex> lock(encQueueMutex);
                encQueueCondition.wait(lock, [] { return !encQueue.empty(); });

                frame = encQueue.front();
                encQueue.pop();
            }

            encoder.encodeFrameFromDataImage(frame);
            delete[] frame;
        }

        // clean up
        encoder.encoderFinishProcess();
    }

    void resetRockPositions(bool reset = true) {
        // rocks can be on x 25 - 600, z from -5 to 5
        std::random_device rd;
        std::mt19937 gen(rd()); // my random generator
        // x plus modifer
        std::uniform_real_distribution<> xRange(10.0, 20.0);
        std::uniform_real_distribution<> zRange(-4.5, 4.5);
        float random_x = 0.0f;
        if (reset) {
            random_x = static_cast<float>(xRange(gen)) + 15;
            models[15].modelMatrix = glm::mat4(1.0f);
            models[16].modelMatrix = glm::mat4(1.0f);
            models[17].modelMatrix = glm::mat4(1.0f);
            models[18].modelMatrix = glm::mat4(1.0f);
            models[19].modelMatrix = glm::mat4(1.0f);

            models[20].modelMatrix = glm::mat4(1.0f);
            models[21].modelMatrix = glm::mat4(1.0f);
            models[22].modelMatrix = glm::mat4(1.0f);
            models[23].modelMatrix = glm::mat4(1.0f);
            models[24].modelMatrix = glm::mat4(1.0f);

            models[25].modelMatrix = glm::mat4(1.0f);
            models[26].modelMatrix = glm::mat4(1.0f);
            models[27].modelMatrix = glm::mat4(1.0f);
            models[28].modelMatrix = glm::mat4(1.0f);
            models[29].modelMatrix = glm::mat4(1.0f);
        }
        else {
            random_x = static_cast<float>(xRange(gen)) + 40;
            for (size_t i{ 15 }; i < 29; ++i) {
                models[i].modelMatrix = models[29].modelMatrix;
                models[i].modelMatrix[3].z = 0.0f;
            }
            models[15].modelMatrix[3].z = 0.0f;

            // PLAY WUHUUUU OR SOMETHING HERE
        }
        
        float random_z = static_cast<float>(zRange(gen));
        models[15].translateModelMatrix(glm::vec3(random_x, 0.0f, random_z)); random_x += static_cast<float>(xRange(gen)); random_z = static_cast<float>(zRange(gen));
        models[16].translateModelMatrix(glm::vec3(random_x, 0.0f, random_z)); random_x += static_cast<float>(xRange(gen)); random_z = static_cast<float>(zRange(gen));
        models[17].translateModelMatrix(glm::vec3(random_x, 0.0f, random_z)); random_x += static_cast<float>(xRange(gen)); random_z = static_cast<float>(zRange(gen));
        models[18].translateModelMatrix(glm::vec3(random_x, 0.0f, random_z)); random_x += static_cast<float>(xRange(gen)); random_z = static_cast<float>(zRange(gen));
        models[19].translateModelMatrix(glm::vec3(random_x, 0.0f, random_z)); random_x += static_cast<float>(xRange(gen)); random_z = static_cast<float>(zRange(gen));

        models[20].translateModelMatrix(glm::vec3(random_x, 0.0f, random_z)); random_x += static_cast<float>(xRange(gen)); random_z = static_cast<float>(zRange(gen));
        models[21].translateModelMatrix(glm::vec3(random_x, 0.0f, random_z)); random_x += static_cast<float>(xRange(gen)); random_z = static_cast<float>(zRange(gen));
        models[22].translateModelMatrix(glm::vec3(random_x, 0.0f, random_z)); random_x += static_cast<float>(xRange(gen)); random_z = static_cast<float>(zRange(gen));
        models[23].translateModelMatrix(glm::vec3(random_x, 0.0f, random_z)); random_x += static_cast<float>(xRange(gen)); random_z = static_cast<float>(zRange(gen));
        models[24].translateModelMatrix(glm::vec3(random_x, 0.0f, random_z)); random_x += static_cast<float>(xRange(gen)); random_z = static_cast<float>(zRange(gen));

        models[25].translateModelMatrix(glm::vec3(random_x, 0.0f, random_z)); random_x += static_cast<float>(xRange(gen)); random_z = static_cast<float>(zRange(gen));
        models[26].translateModelMatrix(glm::vec3(random_x, 0.0f, random_z)); random_x += static_cast<float>(xRange(gen)); random_z = static_cast<float>(zRange(gen));
        models[27].translateModelMatrix(glm::vec3(random_x, 0.0f, random_z)); random_x += static_cast<float>(xRange(gen)); random_z = static_cast<float>(zRange(gen));
        models[28].translateModelMatrix(glm::vec3(random_x, 0.0f, random_z)); random_x += static_cast<float>(xRange(gen)); random_z = static_cast<float>(zRange(gen));
        models[29].translateModelMatrix(glm::vec3(random_x, 0.0f, random_z)); random_x += static_cast<float>(xRange(gen)); random_z = static_cast<float>(zRange(gen));
    }
    
};

void receiveReport() {
    UDPReceive6 receiver;
    receiver.init(51337);

    while (true) {
        char buf[5000];
        double ptime;
        auto ret = receiver.receive(buf, sizeof buf, &ptime);
        buf[ret] = '\0';
        printf("Receiver Report: \n% s", buf);

        std::string stringBuf(buf);
        std::string packetLostString = "packet loss rate: ";
        std::regex format(packetLostString + R"(\s*([\d]+\.[\d]{2}))"); // do not touch anymore

        std::smatch match;
        if (std::regex_search(stringBuf, match, format)) {
            if (match.size() == 2) {
                std::string packetsLostRate = match[1];
                double rate = std::stod(packetsLostRate);

                //std::cout << "This is the lost packet rate: " << rate << "\n";
                if (rate > 5) {
                    //reduces the fps if loss rate is higher than 5%
                    int msDelay = 50;
                    FPS_INTERVAL = std::chrono::milliseconds(msDelay) + FPS_INTERVAL;
                }
                // only goes one way for now

            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

void getMovementUDP() {
    UDPReceive6 receiver;
    receiver.init(51338);

    while (true) {
        char buf[25];
        double ptime;
        auto ret = receiver.receive(buf, sizeof buf, &ptime);
        buf[ret] = '\0';
        //printf("\n\nMovement: \n% s", buf);

        if (buf[0] == 'W') {
            wIsPressed = true;
        }
        else if (buf[0] == 'A') {
            aIsPressed = true;
        }
        else if (buf[0] == 'S') {
            sIsPressed = true;
        }
        else if (buf[0] == 'D') {
            dIsPressed = true;
        }
        else if (buf[0] == 'w') {
            wIsPressed = false;
        }
        else if (buf[0] == 'a') {
            aIsPressed = false;
        }
        else if (buf[0] == 's') {
            sIsPressed = false;
        }
        else if (buf[0] == 'd') {
            dIsPressed = false;
        }
    }
}

int main() {
    startWinsock();
    HelloTriangleApplication app;

    std::thread receiveThread(receiveReport);
    std::thread getMovement(getMovementUDP);

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    receiveThread.join();
    getMovement.join();
    WSACleanup();

    return EXIT_SUCCESS;
}