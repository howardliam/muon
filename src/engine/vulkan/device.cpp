#include "engine/vulkan/device.hpp"

#include <set>
#include <unordered_set>

#include <SDL3/SDL_vulkan.h>

namespace muon {

    /* Debug messenger */
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data) {
        spdlog::error("Validation layer: {}", callback_data->pMessage);

        return VK_FALSE;
    }

    VkResult createDebugUtilsMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *create_info, const VkAllocationCallbacks *allocator, VkDebugUtilsMessengerEXT *debug_messenger) {
        auto instance_proc_addr = vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT");
        const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance_proc_addr);
        if (func != nullptr) {
            return func(instance, create_info, allocator, debug_messenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void destroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks *allocator) {
        auto instance_proc_addr = vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance_proc_addr);
        if (func != nullptr) {
            func(instance, debug_messenger, allocator);
        }
    }

    /* Device class */
    Device::Device(Window &window) : window{window} {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createCommandPool();
    }

    Device::~Device() {
        vkDestroyCommandPool(device, command_pool, nullptr);
        vkDestroyDevice(device, nullptr);

        if (enable_validation_layers) {
            destroyDebugUtilsMessenger(instance, debug_messenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    /* Public functions */
    uint32_t Device::findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
        for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
            if ((type_filter & (1 << i)) &&
                (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        spdlog::error("Failed to find suitable memory type, exiting");
        exit(exitcode::FAILURE);
    }

    VkFormat Device::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        spdlog::error("Failed to find supported format, exiting");
        exit(exitcode::FAILURE);
    }

    void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory) {
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.usage = usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
            spdlog::error("Failed to create vertex buffer, exiting");
            exit(exitcode::FAILURE);
        }

        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = memory_requirements.size;
        alloc_info.memoryTypeIndex = findMemoryType(memory_requirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &alloc_info, nullptr, &buffer_memory) != VK_SUCCESS) {
            spdlog::error("Failed to allocate vertex buffer memory, exiting");
            exit(exitcode::FAILURE);
        }

        vkBindBufferMemory(device, buffer, buffer_memory, 0);
    }

    VkCommandBuffer Device::beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = command_pool;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer;
        vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(command_buffer, &begin_info);
        return command_buffer;
    }

    void Device::endSingleTimeCommands(VkCommandBuffer command_buffer) {
        vkEndCommandBuffer(command_buffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &command_buffer;

        vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphics_queue);

        vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
    }

    void Device::copyBuffer(VkBuffer src_buffer, VkBuffer dest_buffer, VkDeviceSize size) {
        VkCommandBuffer command_buffer = beginSingleTimeCommands();

        VkBufferCopy copy_region{};
        copy_region.srcOffset = 0;
        copy_region.dstOffset = 0;
        copy_region.size = size;
        vkCmdCopyBuffer(command_buffer, src_buffer, dest_buffer, 1, &copy_region);

        endSingleTimeCommands(command_buffer);
    }

    void Device::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer_count) {
        VkCommandBuffer command_buffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layer_count;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        endSingleTimeCommands(command_buffer);
    }

    void Device::createImageWithInfo(const VkImageCreateInfo &image_info, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &image_memory) {
        if (vkCreateImage(device, &image_info, nullptr, &image) != VK_SUCCESS) {
            spdlog::error("Failed to create image, exiting");
            exit(exitcode::FAILURE);
        }

        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(device, image, &memory_requirements);

        VkMemoryAllocateInfo allocate_info{};
        allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate_info.allocationSize = memory_requirements.size;
        allocate_info.memoryTypeIndex = findMemoryType(memory_requirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocate_info, nullptr, &image_memory) != VK_SUCCESS) {
            spdlog::error("Failed to allocate image memory, exiting");
            exit(exitcode::FAILURE);
        }

        if (vkBindImageMemory(device, image, image_memory, 0) != VK_SUCCESS) {
            spdlog::error("Failed to bind image memory, exiting");
            exit(exitcode::FAILURE);
        }
    }

    /* Private functions */
    void Device::createInstance() {
        if (enable_validation_layers && !checkValidationLayerSupport()) {
            spdlog::error("Validation layers requested but not available, exiting");
            exit(exitcode::FAILURE);
        }

        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = defaults::TITLE;
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        auto extensions = getRequiredExtensions();
        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
        if (enable_validation_layers) {
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();

            populateDebugMessengerCreateInfo(debug_create_info);
            create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_create_info;
        } else {
            create_info.enabledLayerCount = 0;
            create_info.pNext = nullptr;
        }

        if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
            spdlog::error("Failed to create instance, exiting");
            exit(exitcode::FAILURE);
        }

        hasSdlRequiredInstanceExtensions();
    }

    void Device::setupDebugMessenger() {
        if constexpr (!enable_validation_layers) {
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT create_info;
        populateDebugMessengerCreateInfo(create_info);

        if (createDebugUtilsMessenger(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
            spdlog::error("Failed to set up debug messenger, exiting");
            exit(exitcode::FAILURE);
        }
    }

    void Device::createSurface() {
        window.createSurface(instance, &surface);
    }

    void Device::pickPhysicalDevice() {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

        if (device_count == 0) {
            spdlog::error("Failed to find GPUs with Vulkan support");
            exit(exitcode::FAILURE);
        }

        spdlog::debug("Device count: {}", device_count);

        std::vector<VkPhysicalDevice> devices(device_count);

        vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

        for (const auto &device : devices) {
            if (isDeviceSuitable(device)) {
                physical_device = device;
                break;
            }
        }

        if (physical_device == VK_NULL_HANDLE) {
            spdlog::error("Failed to find a suitable GPU");
            exit(exitcode::FAILURE);
        }

        vkGetPhysicalDeviceProperties(physical_device, &properties);
        spdlog::debug("Physical device: {}", properties.deviceName);
    }

    void Device::createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physical_device);

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        std::set unique_queue_families = {indices.graphics_family, indices.present_family};

        float queue_priority = 1.0f;
        for (uint32_t queue_family : unique_queue_families) {
            VkDeviceQueueCreateInfo queue_create_info = {};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_family;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(queue_create_info);
        }

        VkPhysicalDeviceFeatures device_features = {};
        device_features.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        create_info.pQueueCreateInfos = queue_create_infos.data();

        create_info.pEnabledFeatures = &device_features;
        create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
        create_info.ppEnabledExtensionNames = device_extensions.data();

        if (enable_validation_layers) {
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();
        } else {
            create_info.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS) {
            spdlog::error("Failed to create logical device, exiting");
            exit(exitcode::FAILURE);
        }

        vkGetDeviceQueue(device, indices.graphics_family, 0, &graphics_queue);
        vkGetDeviceQueue(device, indices.present_family, 0, &present_queue);
    }

    void Device::createCommandPool() {
        QueueFamilyIndices queue_family_indices = getPhysicalQueueFamilies();

        VkCommandPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.queueFamilyIndex = queue_family_indices.graphics_family;
        pool_info.flags =
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pool) != VK_SUCCESS) {
            spdlog::error("Failed to create command pool, exiting");
            exit(exitcode::FAILURE);
        }
    }

    bool Device::isDeviceSuitable(VkPhysicalDevice device) {
        const QueueFamilyIndices indices = findQueueFamilies(device);

        const bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            const SwapChainSupportDetails swap_chain_support = querySwapchainSupport(device);
            swapChainAdequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    std::vector<const char*> Device::getRequiredExtensions() {
        uint32_t extension_count = 0;
        auto sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&extension_count);

        std::vector<const char *> extensions(sdl_extensions, sdl_extensions + extension_count);

        if (enable_validation_layers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool Device::checkValidationLayerSupport() {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        for (const char *layer_name : validation_layers) {
            bool layer_found = false;

            for (const auto &layer_properties : available_layers) {
                if (strcmp(layer_name, layer_properties.layerName) == 0) {
                    layer_found = true;
                    break;
                }
            }

            if (!layer_found) {
                return false;
            }
        }

        return true;
    }

    QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

        int i = 0;
        for (const auto &queue_family : queue_families) {
            if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphics_family = i;
                indices.graphics_family_has_value = true;
            }
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
            if (queue_family.queueCount > 0 && present_support) {
                indices.present_family = i;
                indices.present_family_has_value = true;
            }
            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    void Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &create_info) {
        auto type = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        auto severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        auto message_type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        create_info = {};
        create_info.sType = type;
        create_info.messageSeverity = severity;
        create_info.messageType = message_type;
        create_info.pfnUserCallback = debugCallback;
        create_info.pUserData = nullptr;
    }

    void Device::hasSdlRequiredInstanceExtensions() {
        uint32_t extension_count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> extensions(extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

        spdlog::trace("Available extensions:");
        std::unordered_set<std::string> available;
        for (const auto &extension : extensions) {
            spdlog::trace("\t{}", extension.extensionName);
            available.insert(extension.extensionName);
        }

        spdlog::trace("Required extensions:");
        auto required_extensions = getRequiredExtensions();
        for (const auto &required : required_extensions) {
            spdlog::trace("\t{}", required);
            if (available.find(required) == available.end()) {
                spdlog::error("Missing required SDL extension");
                exit(exitcode::FAILURE);
            }
        }
    }

    bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

        std::vector<VkExtensionProperties> availabile_extensions(extension_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, availabile_extensions.data());

        std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

        for (const auto &extension : availabile_extensions) {
            required_extensions.erase(extension.extensionName);
        }

        return required_extensions.empty();
    }

    SwapChainSupportDetails Device::querySwapchainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

        if (format_count != 0) {
            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
        }

        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

        if (present_mode_count != 0) {
            details.present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
        }
        return details;
    }

}
