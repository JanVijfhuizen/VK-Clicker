#include "pch.h"
#include "Instance.h"
#include <GLFW/glfw3.h>
#include "Vec.h"
#include "Math.h"
#include "ShaderLoader.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData)
{
    std::cerr << "validation: " << callbackData->pMessage << std::endl;
    return VK_FALSE;
}

VkResult InstanceBuilder::CreateDebugUtilsMessengerEXT()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // optional
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(_instance._value, "vkCreateDebugUtilsMessengerEXT");
    return func ? func(_instance._value, &createInfo, nullptr, &_instance._debugMessenger)
        : VK_ERROR_EXTENSION_NOT_PRESENT;
}

void InstanceBuilder::SetCommandPool()
{
    // Create command pool
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = _instance.queueFamily.graphics;

    if (vkCreateCommandPool(_instance._device, &poolInfo, nullptr, &_instance._cmdGraphicsPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics command pool!");
    }

    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = _instance.queueFamily.present;
    if (vkCreateCommandPool(_instance._device, &poolInfo, nullptr, &_instance._cmdPresentPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create present command pool!");
    }

    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = _instance.queueFamily.transfer;
    if (vkCreateCommandPool(_instance._device, &poolInfo, nullptr, &_instance._cmdTransferPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create transfer command pool!");
    }

    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = _instance.queueFamily.compute;
    if (vkCreateCommandPool(_instance._device, &poolInfo, nullptr, &_instance._cmdComputePool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute command pool!");
    }
}

void Instance::RecreateSwapChain(Window& window)
{
    _resolution = window.GetResolution();

    VkSwapchainKHR oldSwapChain = _swapChain;
    if (oldSwapChain)
        DestroySwapChain();

    auto _ = mem::scope(TEMP);

    auto details = TEMP_GetSwapChainSupportDetails();
    auto format = ChooseSwapSurfaceFormat(details.formats);
    SetSwapChainExtent(details.capabilities);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _surface;
    createInfo.minImageCount = details.capabilities.minImageCount + 1;
    if(details.capabilities.maxImageCount != 0)
        createInfo.minImageCount = jv::Min(createInfo.minImageCount, details.capabilities.maxImageCount);

    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = _extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    uint32_t queueFamilyIndices[] = {
        queueFamily.graphics, 
        queueFamily.present
    };

    // If sharing the same queue.
    if (queueFamily.graphics != queueFamily.present) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = details.capabilities.currentTransform;
    if (!(details.capabilities.supportedTransforms & details.capabilities.currentTransform))
        createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    auto flags = details.capabilities.supportedCompositeAlpha;
    if (!(flags & compositeAlpha)) {
        if (flags & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
            compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
        else if (flags & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
            compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
        else if (flags & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
            compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }
    createInfo.compositeAlpha = compositeAlpha;
 
    createInfo.presentMode = ChooseSwapPresentMode(details.presentModes);
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapChain;

    auto result = vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain);
    if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to create Swap Chain!");

    CreateImages(format, oldSwapChain);
    CreateRenderPass(format);
    CreateFrameBuffers(oldSwapChain);
}

Instance::SwapChainSupportDetails Instance::TEMP_GetSwapChainSupportDetails()
{
    SwapChainSupportDetails details{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        _physicalDevice, _surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        _physicalDevice, _surface, &formatCount, nullptr);

    if (formatCount != 0) {
        auto& arr = details.formats = mem::Arr<VkSurfaceFormatKHR>(TEMP, formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            _physicalDevice, _surface, &formatCount, arr.ptr());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        _physicalDevice, _surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        auto& arr = details.presentModes = mem::Arr<VkPresentModeKHR>(TEMP, presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            _physicalDevice, _surface, &presentModeCount, arr.ptr());
    }

    return details;
}

VkSurfaceFormatKHR Instance::ChooseSwapSurfaceFormat(const mem::Arr<VkSurfaceFormatKHR>& formats)
{
    VkSurfaceFormatKHR format = formats[0];
    formats.iterb([&format](auto& f, auto) {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB &&
            f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            format = f;
            return false;
        }
        return true;
        });
    return format;
}

VkPresentModeKHR Instance::ChooseSwapPresentMode(const mem::Arr<VkPresentModeKHR>& modes)
{
    VkPresentModeKHR mode{};
    VkPresentModeKHR preferred;
    switch (_preferredPresentMode)
    {
    case PresentMode::immediate:
        preferred = VK_PRESENT_MODE_IMMEDIATE_KHR;
        break;
    case PresentMode::mailbox:
        preferred = VK_PRESENT_MODE_MAILBOX_KHR;
        break;
    case PresentMode::fifo:
    default:
        preferred = VK_PRESENT_MODE_FIFO_KHR;
        break;
    }

    modes.iterb([&mode, preferred](auto& m, auto) {
        if (m == preferred)
        {
            mode = m;
            return false;
        }
        return true;
        });
    return mode;
}

void Instance::SetSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
        _extent = capabilities.currentExtent;

    VkExtent2D actualExtent = { _resolution.x, _resolution.y };

    actualExtent.width = jv::Clamp(
        actualExtent.width,
        capabilities.minImageExtent.width,
        capabilities.maxImageExtent.width);

    actualExtent.height = jv::Clamp(
        actualExtent.height,
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height);
    _extent = actualExtent;
}

void Instance::CreateImages(VkSurfaceFormatKHR format, VkSwapchainKHR oldSwapChain)
{
    // Create images.
    uint32_t imageCount = _images.length();
    if (!oldSwapChain)
    {
        vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);
        _images = mem::Arr<VkImage>(_arena, imageCount);
        _views = mem::Arr<VkImageView>(_arena, imageCount);
    }

    vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _images.ptr());

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    for (uint32_t i = 0; i < _images.length(); i++)
    {
        viewInfo.image = _images[i];
        vkCreateImageView(_device, &viewInfo, nullptr, &_views[i]);
    }
}

void Instance::CreateRenderPass(VkSurfaceFormatKHR format)
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = format.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS)
        throw std::runtime_error("Failed to create render pass!");
}

void Instance::CreateFrameBuffers(VkSwapchainKHR oldSwapChain)
{
    if (!oldSwapChain)
        _frameBuffers = mem::Arr<VkFramebuffer>(_arena, _images.length());

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = _renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.width = _extent.width;
    framebufferInfo.height = _extent.height;
    framebufferInfo.layers = 1;

    for (uint32_t i = 0; i < _images.length(); i++)
    {
        VkImageView attachments[] = { _views[i] };
        framebufferInfo.pAttachments = attachments;

        if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_frameBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer!");
    }
}

void Instance::DestroySwapChain()
{
    for (uint32_t i = 0; i < _frameBuffers.length(); i++)
        vkDestroyFramebuffer(_device, _frameBuffers[i], nullptr);
    vkDestroyRenderPass(_device, _renderPass, nullptr);
    for (uint32_t i = 0; i < _images.length(); i++)
        vkDestroyImageView(_device, _views[i], nullptr);
    vkDestroySwapchainKHR(_device, _swapChain, nullptr);
}

void InstanceBuilder::CreateDefaultDescriptors()
{
    VkDescriptorSetLayoutBinding colorUboBinding{};
    colorUboBinding.binding = 0;
    colorUboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    colorUboBinding.descriptorCount = 1;
    colorUboBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    colorUboBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo setLayoutInfo{};
    setLayoutInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setLayoutInfo.bindingCount = 1;
    setLayoutInfo.pBindings = &colorUboBinding;
    setLayoutInfo.flags = 0;

    if (vkCreateDescriptorSetLayout(_instance._device, &setLayoutInfo, nullptr, &_instance._descriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create default Descriptor set layout!");
}

void InstanceBuilder::CreateDefaultPipelineLayout()
{
    VkPushConstantRange pushConstantSize{};
    pushConstantSize.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantSize.size = sizeof(DefPushConstant);
    pushConstantSize.offset = 0;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &_instance._descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantSize;
    vkCreatePipelineLayout(_instance._device, &pipelineLayoutInfo, nullptr, &_instance._pipelineLayout);
}

void InstanceBuilder::CreateDefaultPipeline()
{
    auto frag = LoadShader(_instance._device, _defaultFragPath);
    auto vert = LoadShader(_instance._device, _defaultVertPath);

    VkPipelineShaderStageCreateInfo vertStage{};
    vertStage.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vert;
    vertStage.pName = "main";
    vertStage.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragStage{};
    fragStage.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = frag;
    fragStage.pName = "main";
    fragStage.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo stages[] = {
        vertStage,
        fragStage
    };

    VkVertexInputBindingDescription bindingDesc{};
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(DefVertex);
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attrDesc{};
    attrDesc.location = 0;
    attrDesc.binding = 0;
    attrDesc.format = VK_FORMAT_R32G32_SFLOAT;
    attrDesc.offset = offsetof(DefVertex, pos);

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.pVertexBindingDescriptions = &bindingDesc;
    vertexInput.vertexAttributeDescriptionCount = 1;
    vertexInput.pVertexAttributeDescriptions = &attrDesc;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    auto extent = _instance._extent;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = stages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.layout = _instance._pipelineLayout;
    pipelineInfo.renderPass = _instance._renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.pViewportState = &viewportState;

    vkDestroyShaderModule(_instance._device, frag, nullptr);
    vkDestroyShaderModule(_instance._device, vert, nullptr);
}

Instance InstanceBuilder::Build(ARENA arena, Window& window)
{
    assert(arena != TEMP);

    _instance = {};
    _instance._arena = arena;
    _instance._resolution = window.GetResolution();
    _instance._preferredPresentMode = _preferredPresentMode;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = _name;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = _name;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto _ = mem::scope(TEMP);

    auto layers = mem::Arr<const char*>(TEMP, _validationLayers.length() + 1);
    layers[0] = "VK_LAYER_KHRONOS_validation";
    layers.put(1, _validationLayers);
    createInfo.enabledLayerCount = layers.length();
    createInfo.ppEnabledLayerNames = layers.ptr();

    auto extensions = mem::Arr<const char*>(TEMP, _windowingExtensionsCount + 1);
    extensions[0] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    extensions.put(1, _windowingExtensions, _windowingExtensionsCount);
    createInfo.enabledExtensionCount = extensions.length();
    createInfo.ppEnabledExtensionNames = extensions.ptr();

    if (vkCreateInstance(&createInfo, nullptr, &_instance._value) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan instance!");

    if (glfwCreateWindowSurface(_instance._value, window.Ptr(), nullptr, &_instance._surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface!");

    SetPhysicalDevice();
    SetLogicalDevice();
    auto result = CreateDebugUtilsMessengerEXT();
    if(result != VK_SUCCESS)
        throw std::runtime_error("Failed to create Debug Messenger!");

    CreateDefaultDescriptors();
    CreateDefaultPipelineLayout();
    CreateDefaultPipeline();
    
    SetCommandPool();
    _instance.RecreateSwapChain(window);
    return _instance;
}

InstanceBuilder& InstanceBuilder::SetName(const char* name)
{
    _name = name;
    return *this;
}

InstanceBuilder& InstanceBuilder::AddGLFWSupport()
{
    _windowingExtensions = glfwGetRequiredInstanceExtensions(&_windowingExtensionsCount);
    return *this;
}

InstanceBuilder& InstanceBuilder::SetValidationLayers(mem::Arr<const char*> layers)
{
    _validationLayers = layers;
    return *this;
}

InstanceBuilder& InstanceBuilder::SetPreferredPresentMode(PresentMode mode)
{
    _preferredPresentMode = mode;
    return *this;
}

InstanceBuilder& InstanceBuilder::SetDefaultVertPath(const char* path)
{
    _defaultVertPath = path;
    return *this;
}

InstanceBuilder& InstanceBuilder::SetDefaultFragPath(const char* path)
{
    _defaultFragPath = path;
    return *this;
}

mem::Arr<VkPhysicalDevice> InstanceBuilder::GetPhysicalDevices()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance._value, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("No Vulkan GPUs found!");
    }

    auto arr = mem::Arr<VkPhysicalDevice>(TEMP, deviceCount);
    vkEnumeratePhysicalDevices(_instance._value, &deviceCount, arr.ptr());
    return arr;
}

void InstanceBuilder::SetPhysicalDevice()
{
    struct Rateable {
        VkPhysicalDevice device;
        uint32_t rating = 0;
    };

    auto _ = mem::scope(TEMP);
    auto devices = GetPhysicalDevices();
    auto requiredExtensions = mem::Arr<const char*>(TEMP, 1);
    requiredExtensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

    auto rateables = mem::Vec<Rateable>(TEMP, devices.length());
    devices.iter([&rateables](auto& device, auto i) {
        auto& r = rateables.add() = {};
        r.device = device;
        });

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    auto availableExtensions = mem::Arr<VkExtensionProperties>(TEMP, extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.ptr());

    for (int32_t i = rateables.count() - 1; i >= 0; i--)
    {
        auto& rateable = rateables[i];

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(rateable.device, &features);

        if (!features.samplerAnisotropy)
            continue;

        // Check if this CPU can support ALL required extensions.
        bool valid = true;
        for (uint32_t j = 0; j < requiredExtensions.length(); j++)
        {
            auto required = requiredExtensions[j];
            bool found = false;

            // Can use a hashset for this but this is a small project anyway.
            for (uint32_t j = 0; j < availableExtensions.length(); j++)
            {
                auto available = availableExtensions[j];
                if (strcmp(required, available.extensionName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                valid = false;
                break;
            }
        }

        VkPhysicalDeviceProperties properties;
        
        vkGetPhysicalDeviceProperties(rateable.device, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            rateable.rating += 1e4;

        // Larger textures are better
        rateable.rating += properties.limits.maxImageDimension2D;

        // VRAM size
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(rateable.device, &memProps);

        VkDeviceSize localHeapSize = 0;
        for (uint32_t i = 0; i < memProps.memoryHeapCount; i++)
        {
            if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                localHeapSize += memProps.memoryHeaps[i].size;
        }

        rateable.rating += static_cast<uint32_t>(localHeapSize / (1024 * 1024 * 1024)); // GB
    }

    // Sort by rating.
    rateables.sort([](Rateable& a, Rateable& b) {
        return a.rating > b.rating;
    });
    _instance._physicalDevice = rateables[0].device;
}

void InstanceBuilder::SetLogicalDevice()
{
    // Create queues.
    float queuePriority = 1;
    auto queueFamily = _instance.queueFamily = GetQueueFamily();

    // Very hacky but it's whatever.
    auto familyVec = mem::Vec<uint32_t>(TEMP, 2);
    familyVec.add() = queueFamily.graphics;
    if(familyVec[0] != queueFamily.present)
        familyVec.add() = queueFamily.present;

    auto queueCreateInfos = mem::Arr<VkDeviceQueueCreateInfo>(TEMP, familyVec.count());
    familyVec.arr().iter([&queueCreateInfos, &queueFamily, &queuePriority](auto& family, auto i) {
        auto& info = queueCreateInfos[i] = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = family;
        info.queueCount = 1;
        info.pQueuePriorities = &queuePriority;
        });

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = queueCreateInfos.length();
    createInfo.pQueueCreateInfos = queueCreateInfos.ptr();

    const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    if (vkCreateDevice(_instance._physicalDevice, &createInfo, nullptr, &_instance._device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(_instance._device, queueFamily.graphics, 0, &_instance._graphicsQueue);

    if (queueFamily.graphics != queueFamily.present)
        vkGetDeviceQueue(_instance._device, queueFamily.present, 0, &_instance._presentQueue);
    else
        _instance._presentQueue = _instance._graphicsQueue;
}

QueueFamily InstanceBuilder::GetQueueFamily()
{
    QueueFamily family;

    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_instance._physicalDevice, &familyCount, nullptr);

    auto scope = mem::scope(TEMP);
    auto arr = mem::Arr<VkQueueFamilyProperties>(TEMP, familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_instance._physicalDevice, &familyCount, arr.ptr());

    uint32_t i = 0;
    arr.iterb([&family, &i, this](VkQueueFamilyProperties& current, auto) {
        if (current.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            family.graphics = i;
        }

        if (current.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            family.transfer = i;
        }

        if (current.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            family.compute = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(_instance._physicalDevice, i, _instance._surface, &presentSupport);
        if (presentSupport) {
            family.present = i;
        }

        if (family.Complete())
            return false;
        i++;
        return true;
        });
    
    return family;
}

void Instance::OnScopeClear()
{
    vkDeviceWaitIdle(_device);

    DestroySwapChain();

    vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);

    vkDestroyCommandPool(_device, _cmdComputePool, nullptr);
    vkDestroyCommandPool(_device, _cmdTransferPool, nullptr);
    vkDestroyCommandPool(_device, _cmdPresentPool, nullptr);
    vkDestroyCommandPool(_device, _cmdGraphicsPool, nullptr);

    auto DestroyDebugUtilsMessengerEXT =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_value, "vkDestroyDebugUtilsMessengerEXT");

    if (_debugMessenger != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(_value, _debugMessenger, nullptr);
    }

    vkDestroyDevice(_device, nullptr);
    vkDestroySurfaceKHR(_value, _surface, nullptr);
    vkDestroyInstance(_value, nullptr);
}

bool QueueFamily::Complete()
{
    for (uint32_t i = 0; i < sizeof(queues) / sizeof(uint32_t); i++)
    {
        if (queues[i] == -1)
            return false;
    }
    return true;
}
