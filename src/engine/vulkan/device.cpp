#include "engine/vulkan/device.hpp"

#include <set>
#include <unordered_set>

#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_to_string.hpp>

#include "utils/defaults.hpp"
#include "utils/exitcode.hpp"

namespace muon {

    /* Debug messenger */
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data) {
        spdlog::error("Validation layer: {}", callback_data->pMessage);

        return VK_FALSE;
    }

    vk::Result createDebugUtilsMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *create_info, const VkAllocationCallbacks *allocator, VkDebugUtilsMessengerEXT *debug_messenger) {
        auto instance_proc_addr = vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT");
        const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance_proc_addr);
        if (func != nullptr) {
            VkResult result = func(instance, create_info, allocator, debug_messenger);
            return static_cast<vk::Result>(result);
        } else {
            return vk::Result::eErrorExtensionNotPresent;
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
        device.destroyCommandPool(command_pool, nullptr);
        device.destroy();

        if (enable_validation_layers) {
            destroyDebugUtilsMessenger(instance, debug_messenger, nullptr);
        }

        instance.destroySurfaceKHR(surface, nullptr);
        instance.destroy();
    }

    /* Public functions */
    uint32_t Device::findMemoryType(uint32_t type_filter, vk::MemoryPropertyFlags properties) {
        vk::PhysicalDeviceMemoryProperties memory_properties;
        physical_device.getMemoryProperties(&memory_properties);
        for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
            if ((type_filter & (1 << i)) &&
                (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        spdlog::error("Failed to find suitable memory type, exiting");
        exit(exitcode::FAILURE);
    }

    vk::Format Device::findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
        for (vk::Format format : candidates) {
            vk::FormatProperties props;
            physical_device.getFormatProperties(format, &props);

            if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        spdlog::error("Failed to find supported format, exiting");
        exit(exitcode::FAILURE);
    }

    void Device::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &buffer_memory) {
        vk::BufferCreateInfo buffer_info{};
        buffer_info.sType = vk::StructureType::eBufferCreateInfo;
        buffer_info.size = size;
        buffer_info.usage = usage;
        buffer_info.sharingMode = vk::SharingMode::eExclusive;

        if (device.createBuffer(&buffer_info, nullptr, &buffer) != vk::Result::eSuccess) {
            spdlog::error("Failed to create vertex buffer, exiting");
            exit(exitcode::FAILURE);
        }

        vk::MemoryRequirements memory_requirements;
        device.getBufferMemoryRequirements(buffer, &memory_requirements);

        vk::MemoryAllocateInfo alloc_info{};
        alloc_info.sType = vk::StructureType::eMemoryAllocateInfo;
        alloc_info.allocationSize = memory_requirements.size;
        alloc_info.memoryTypeIndex = findMemoryType(memory_requirements.memoryTypeBits, properties);

        if (device.allocateMemory(&alloc_info, nullptr, &buffer_memory) != vk::Result::eSuccess) {
            spdlog::error("Failed to allocate vertex buffer memory, exiting");
            exit(exitcode::FAILURE);
        }

        vkBindBufferMemory(device, buffer, buffer_memory, 0);
    }

    vk::CommandBuffer Device::beginSingleTimeCommands() {
        vk::CommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = vk::StructureType::eCommandBufferAllocateInfo;
        alloc_info.level = vk::CommandBufferLevel::ePrimary;
        alloc_info.commandPool = command_pool;
        alloc_info.commandBufferCount = 1;

        vk::CommandBuffer command_buffer;
        device.allocateCommandBuffers(&alloc_info, &command_buffer);

        vk::CommandBufferBeginInfo begin_info{};
        begin_info.sType = vk::StructureType::eCommandBufferBeginInfo;
        begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        command_buffer.begin(&begin_info);
        return command_buffer;
    }

    void Device::endSingleTimeCommands(vk::CommandBuffer command_buffer) {
        vkEndCommandBuffer(command_buffer);

        vk::SubmitInfo submitInfo{};
        submitInfo.sType = vk::StructureType::eSubmitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &command_buffer;

        graphics_queue.submit(1, &submitInfo, VK_NULL_HANDLE);
        graphics_queue.waitIdle();

        device.freeCommandBuffers(command_pool, 1, &command_buffer);
    }

    void Device::copyBuffer(vk::Buffer src_buffer, vk::Buffer dest_buffer, vk::DeviceSize size) {
        vk::CommandBuffer command_buffer = beginSingleTimeCommands();

        vk::BufferCopy copy_region{};
        copy_region.srcOffset = 0;
        copy_region.dstOffset = 0;
        copy_region.size = size;
        command_buffer.copyBuffer(src_buffer, dest_buffer, 1, &copy_region);

        endSingleTimeCommands(command_buffer);
    }

    void Device::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layer_count) {
        vk::CommandBuffer command_buffer = beginSingleTimeCommands();

        vk::BufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layer_count;

        region.setImageOffset({0, 0, 0});
        region.setImageExtent({width, height, 1});

        command_buffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
        endSingleTimeCommands(command_buffer);
    }

    void Device::createImageWithInfo(const vk::ImageCreateInfo &image_info, vk::MemoryPropertyFlags properties, vk::Image &image, vk::DeviceMemory &image_memory) {
        if (device.createImage(&image_info, nullptr, &image) != vk::Result::eSuccess) {
            spdlog::error("Failed to create image, exiting");
            exit(exitcode::FAILURE);
        }

        vk::MemoryRequirements memory_requirements;
        device.getImageMemoryRequirements(image, &memory_requirements);

        vk::MemoryAllocateInfo allocate_info{};
        allocate_info.sType = vk::StructureType::eMemoryAllocateInfo;
        allocate_info.allocationSize = memory_requirements.size;
        allocate_info.memoryTypeIndex = findMemoryType(memory_requirements.memoryTypeBits, properties);

        if (device.allocateMemory(&allocate_info, nullptr, &image_memory) != vk::Result::eSuccess) {
            spdlog::error("Failed to allocate image memory, exiting");
            exit(exitcode::FAILURE);
        }

        device.bindImageMemory(image, image_memory, 0);
        // if (device.bindImageMemory(image, image_memory, 0) != vk::Result::eSuccess) {
        //     spdlog::error("Failed to bind image memory, exiting");
        //     exit(exitcode::FAILURE);
        // }
    }

    /* Private functions */
    void Device::createInstance() {
        if (enable_validation_layers && !checkValidationLayerSupport()) {
            spdlog::error("Validation layers requested but not available, exiting");
            exit(exitcode::FAILURE);
        }

        vk::ApplicationInfo app_info = {};
        app_info.sType = vk::StructureType::eApplicationInfo;
        app_info.pApplicationName = defaults::window::TITLE.c_str();
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "Muon";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;

        vk::InstanceCreateInfo create_info = {};
        create_info.sType = vk::StructureType::eInstanceCreateInfo;
        create_info.pApplicationInfo = &app_info;

        auto extensions = getRequiredExtensions();
        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        vk::DebugUtilsMessengerCreateInfoEXT debug_create_info;
        if (enable_validation_layers) {
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();

            populateDebugMessengerCreateInfo(debug_create_info);
            create_info.pNext = (vk::DebugUtilsMessengerCreateInfoEXT *)&debug_create_info;
        } else {
            create_info.enabledLayerCount = 0;
            create_info.pNext = nullptr;
        }

        if (vk::createInstance(&create_info, nullptr, &instance) != vk::Result::eSuccess) {
            spdlog::error("Failed to create instance, exiting");
            exit(exitcode::FAILURE);
        }

        hasSdlRequiredInstanceExtensions();
    }

    void Device::setupDebugMessenger() {
        if constexpr (!enable_validation_layers) {
            return;
        }

        vk::DebugUtilsMessengerCreateInfoEXT create_info;
        populateDebugMessengerCreateInfo(create_info);

        vk::Result result = createDebugUtilsMessenger(
            static_cast<VkInstance>(instance),
            reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT *>(&create_info),
            nullptr,
            reinterpret_cast<VkDebugUtilsMessengerEXT *>(&debug_messenger)
        );
        if (result != vk::Result::eSuccess) {
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

        std::vector<vk::PhysicalDevice> devices(device_count);

        instance.enumeratePhysicalDevices(&device_count, devices.data());

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

        physical_device.getProperties(&properties);
        spdlog::debug("Physical device: {}", properties.deviceName.data());
    }

    void Device::createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physical_device);

        std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
        std::set unique_queue_families = {indices.graphics_family, indices.present_family};

        float queue_priority = 1.0f;
        for (uint32_t queue_family : unique_queue_families) {
            vk::DeviceQueueCreateInfo queue_create_info = {};
            queue_create_info.sType = vk::StructureType::eDeviceQueueCreateInfo;
            queue_create_info.queueFamilyIndex = queue_family;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(queue_create_info);
        }

        vk::PhysicalDeviceFeatures device_features = {};
        device_features.samplerAnisotropy = VK_TRUE;

        vk::DeviceCreateInfo create_info = {};
        create_info.sType = vk::StructureType::eDeviceCreateInfo;

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

        if (physical_device.createDevice(&create_info, nullptr, &device) != vk::Result::eSuccess) {
            spdlog::error("Failed to create logical device, exiting");
            exit(exitcode::FAILURE);
        }

        device.getQueue(indices.graphics_family, 0, &graphics_queue);
        device.getQueue(indices.present_family, 0, &present_queue);
    }

    void Device::createCommandPool() {
        QueueFamilyIndices queue_family_indices = getPhysicalQueueFamilies();

        vk::CommandPoolCreateInfo pool_info = {};
        pool_info.sType = vk::StructureType::eCommandPoolCreateInfo;
        pool_info.queueFamilyIndex = queue_family_indices.graphics_family;
        pool_info.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

        if (device.createCommandPool(&pool_info, nullptr, &command_pool) != vk::Result::eSuccess) {
            spdlog::error("Failed to create command pool, exiting");
            exit(exitcode::FAILURE);
        }
    }

    bool Device::isDeviceSuitable(vk::PhysicalDevice device) {
        const QueueFamilyIndices indices = findQueueFamilies(device);

        const bool extensions_supported = checkDeviceExtensionSupport(device);

        bool swapchain_adequate = false;
        if (extensions_supported) {
            const SwapchainSupportDetails swap_chain_support = querySwapchainSupport(device);
            swapchain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
        }

        vk::PhysicalDeviceFeatures supported_features;
        device.getFeatures(&supported_features);

        return indices.isComplete() && extensions_supported && swapchain_adequate && supported_features.samplerAnisotropy;
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
        // uint32_t layer_count;
        // vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        // std::vector<vk::LayerProperties> available_layers(layer_count);
        // vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        std::vector<vk::LayerProperties> available_layers = vk::enumerateInstanceLayerProperties();

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

    QueueFamilyIndices Device::findQueueFamilies(vk::PhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queue_family_count = 0;
        device.getQueueFamilyProperties(&queue_family_count, nullptr);

        std::vector<vk::QueueFamilyProperties> queue_families(queue_family_count);
        device.getQueueFamilyProperties(&queue_family_count, queue_families.data());


        int32_t i = 0;
        for (const auto &queue_family : queue_families) {
            if (queue_family.queueCount > 0 && queue_family.queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.graphics_family = i;
                indices.graphics_family_has_value = true;
            }
            vk::Bool32 present_support = false;
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

    void Device::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &create_info) {
        auto type = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;
        auto severity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        auto message_type = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

        create_info = vk::DebugUtilsMessengerCreateInfoEXT{};
        create_info.sType = type;
        create_info.messageSeverity = severity;
        create_info.messageType = message_type;
        create_info.pfnUserCallback = debugCallback;
        create_info.pUserData = nullptr;
    }

    void Device::hasSdlRequiredInstanceExtensions() {
        // uint32_t extension_count = 0;
        // vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

        // std::vector<vk::ExtensionProperties> extensions(extension_count);
        // vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

        std::vector<vk::ExtensionProperties> extensions = vk::enumerateInstanceExtensionProperties();

        spdlog::trace("Available extensions:");
        std::unordered_set<std::string> available;
        for (const auto &extension : extensions) {
            spdlog::trace("\t{}", extension.extensionName.data());
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

    bool Device::checkDeviceExtensionSupport(vk::PhysicalDevice device) {
        uint32_t extension_count;
        device.enumerateDeviceExtensionProperties(nullptr, &extension_count, nullptr);

        std::vector<vk::ExtensionProperties> availabile_extensions(extension_count);
        device.enumerateDeviceExtensionProperties(nullptr, &extension_count, availabile_extensions.data());

        std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

        for (const auto &extension : availabile_extensions) {
            required_extensions.erase(extension.extensionName);
        }

        return required_extensions.empty();
    }

    SwapchainSupportDetails Device::querySwapchainSupport(vk::PhysicalDevice device) {
        SwapchainSupportDetails details;
        device.getSurfaceCapabilitiesKHR(surface, &details.capabilities);

        uint32_t format_count;
        device.getSurfaceFormatsKHR(surface, &format_count, nullptr);

        if (format_count != 0) {
            details.formats.resize(format_count);
            device.getSurfaceFormatsKHR(surface, &format_count, details.formats.data());
        }

        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

        if (present_mode_count != 0) {
            details.present_modes.resize(present_mode_count);
            device.getSurfacePresentModesKHR(surface, &present_mode_count, details.present_modes.data());
        }
        return details;
    }

}
