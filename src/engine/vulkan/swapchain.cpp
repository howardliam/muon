#include "engine/vulkan/swapchain.hpp"

namespace muon {

    Swapchain::Swapchain(Device &device, VkExtent2D window_extent) : device{device}, window_extent{window_extent} {
        init();
    }

    Swapchain::Swapchain(Device &device, VkExtent2D window_extent, std::shared_ptr<Swapchain> previous) : device{device}, window_extent{window_extent}, old_swapchain{previous} {
        init();
        old_swapchain = nullptr;
    }

    Swapchain::~Swapchain() {
        for (auto image_view : swapchain_image_views) {
            vkDestroyImageView(device.getDevice(), image_view, nullptr);
        }
        swapchain_image_views.clear();

        if (swapchain != nullptr) {
            vkDestroySwapchainKHR(device.getDevice(), swapchain, nullptr);
            swapchain = nullptr;
        }

        for (int i = 0; i < depth_images.size(); i++) {
            vkDestroyImageView(device.getDevice(), depth_image_views[i], nullptr);
            vkDestroyImage(device.getDevice(), depth_images[i], nullptr);
            vkFreeMemory(device.getDevice(), depth_image_memories[i], nullptr);
        }

        for (auto framebuffer : swapchain_framebuffers) {
            vkDestroyFramebuffer(device.getDevice(), framebuffer, nullptr);
        }

        vkDestroyRenderPass(device.getDevice(), render_pass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device.getDevice(), render_finished_semaphores[i], nullptr);
            vkDestroySemaphore(device.getDevice(), image_available_semaphores[i], nullptr);
            vkDestroyFence(device.getDevice(), in_flight_fences[i], nullptr);
        }
    }

    VkFormat Swapchain::findDepthFormat() {
        auto candidates = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
        auto tiling = VK_IMAGE_TILING_OPTIMAL;
        auto features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

        return device.findSupportedFormat(candidates, tiling, features);
    }

    VkResult Swapchain::acquireNextImage(uint32_t* image_index) {
        vkWaitForFences(device.getDevice(), 1, &in_flight_fences[current_frame],
            VK_TRUE, std::numeric_limits<uint64_t>::max());

        VkResult result = vkAcquireNextImageKHR(device.getDevice(), swapchain, std::numeric_limits<uint64_t>::max(),image_available_semaphores[current_frame], VK_NULL_HANDLE, image_index);

        return result;
    }

    VkResult Swapchain::submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* image_index) {
        if (images_in_flight[*image_index] != VK_NULL_HANDLE) {
            vkWaitForFences(device.getDevice(), 1, &images_in_flight[*image_index], VK_TRUE, UINT64_MAX);
        }
        images_in_flight[*image_index] = in_flight_fences[current_frame];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore wait_semaphores[] = {image_available_semaphores[current_frame]};
        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = wait_semaphores;
        submitInfo.pWaitDstStageMask = wait_stages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        VkSemaphore signal_semaphores[] = {render_finished_semaphores[current_frame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signal_semaphores;

        vkResetFences(device.getDevice(), 1, &in_flight_fences[current_frame]);
        if (vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, in_flight_fences[current_frame]) != VK_SUCCESS) {
            spdlog::error("Failed to submit draw command buffer");
            exit(exitcode::FAILURE);
        }

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = signal_semaphores;

        VkSwapchainKHR swapchains[] = {swapchain};
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swapchains;

        present_info.pImageIndices = image_index;

        auto result = vkQueuePresentKHR(device.getPresentQueue(), &present_info);

        current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
    }

    bool Swapchain::compareSwapFormats(const Swapchain &swapchain) const {
        return swapchain.swapchain_depth_format == swapchain_depth_format
            && swapchain.swapchain_image_format == swapchain_image_format;
    }

    void Swapchain::init() {
        createSwapchain();
        createImageViews();
        createDepthResources();
        createRenderPass();
        createFramebuffers();
        createSyncObjects();
    }

    void Swapchain::createSwapchain() {
        SwapChainSupportDetails swapchain_support = device.getSwapchainSupport();

        VkSurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(swapchain_support.formats);
        VkPresentModeKHR present_mode = chooseSwapPresentMode(swapchain_support.present_modes);
        VkExtent2D extent = chooseSwapExtent(swapchain_support.capabilities);

        uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
        if (swapchain_support.capabilities.maxImageCount > 0 &&
            image_count > swapchain_support.capabilities.maxImageCount) {
            image_count = swapchain_support.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = device.getSurface();

        create_info.minImageCount = image_count;
        create_info.imageFormat = surface_format.format;
        create_info.imageColorSpace = surface_format.colorSpace;
        create_info.imageExtent = extent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = device.getPhysicalQueueFamilies();
        uint32_t queue_family_indices[] = {indices.graphics_family, indices.present_family};

        if (indices.graphics_family != indices.present_family) {
            create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = queue_family_indices;
        } else {
            create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            create_info.queueFamilyIndexCount = 0;
            create_info.pQueueFamilyIndices = nullptr;
        }

        create_info.preTransform = swapchain_support.capabilities.currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        create_info.presentMode = present_mode;
        create_info.clipped = VK_TRUE;

        create_info.oldSwapchain = old_swapchain == nullptr ? VK_NULL_HANDLE : old_swapchain->swapchain;

        if (vkCreateSwapchainKHR(device.getDevice(), &create_info, nullptr, &swapchain) != VK_SUCCESS) {
            spdlog::error("Failed to create swapchain");
            exit(exitcode::FAILURE);
        }

        vkGetSwapchainImagesKHR(device.getDevice(), swapchain, &image_count, nullptr);
        swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(device.getDevice(), swapchain, &image_count, swapchain_images.data());

        swapchain_image_format = surface_format.format;
        swapchain_extent = extent;
    }

    void Swapchain::createImageViews() {
        swapchain_image_views.resize(swapchain_images.size());
        for (size_t i = 0; i < swapchain_images.size(); i++) {
            VkImageViewCreateInfo view_info{};
            view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_info.image = swapchain_images[i];
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_info.format = swapchain_image_format;
            view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.baseArrayLayer = 0;
            view_info.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device.getDevice(), &view_info, nullptr, &swapchain_image_views[i]) !=
                VK_SUCCESS) {
                spdlog::error("Failed to create texture image view");
                exit(exitcode::FAILURE);
            }
        }
    }

    void Swapchain::createDepthResources() {
        VkFormat depth_format = findDepthFormat();
        swapchain_depth_format = depth_format;
        VkExtent2D swapchain_extent = getSwapchainExtent();

        depth_images.resize(getImageCount());
        depth_image_memories.resize(getImageCount());
        depth_image_views.resize(getImageCount());

        for (int i = 0; i < depth_images.size(); i++) {
            VkImageCreateInfo image_info{};
            image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_info.imageType = VK_IMAGE_TYPE_2D;
            image_info.extent.width = swapchain_extent.width;
            image_info.extent.height = swapchain_extent.height;
            image_info.extent.depth = 1;
            image_info.mipLevels = 1;
            image_info.arrayLayers = 1;
            image_info.format = depth_format;
            image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            image_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_info.flags = 0;

            device.createImageWithInfo(image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_images[i], depth_image_memories[i]);

            VkImageViewCreateInfo view_info{};
            view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_info.image = depth_images[i];
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_info.format = depth_format;
            view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.baseArrayLayer = 0;
            view_info.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device.getDevice(), &view_info, nullptr, &depth_image_views[i]) != VK_SUCCESS) {
                spdlog::error("Failed to create texture image view");
                exit(exitcode::FAILURE);
            }
        }
    }

    void Swapchain::createRenderPass() {
        VkAttachmentDescription depth_attachment{};
        depth_attachment.format = findDepthFormat();
        depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_attachment_ref{};
        depth_attachment_ref.attachment = 1;
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colour_attachment = {};
        colour_attachment.format = getSwapchainImageFormat();
        colour_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colour_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colour_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colour_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colour_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colour_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colour_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colour_attachement_ref = {};
        colour_attachement_ref.attachment = 0;
        colour_attachement_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colour_attachement_ref;
        subpass.pDepthStencilAttachment = &depth_attachment_ref;

        VkSubpassDependency dependency = {};
        dependency.dstSubpass = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcAccessMask = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

        std::array<VkAttachmentDescription, 2> attachments = {colour_attachment, depth_attachment};
        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_info.pAttachments = attachments.data();
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &dependency;

        if (vkCreateRenderPass(device.getDevice(), &render_pass_info, nullptr, &render_pass) != VK_SUCCESS) {
            spdlog::error("Failed to create render pass");
            exit(exitcode::FAILURE);
        }
    }

    void Swapchain::createFramebuffers() {
        swapchain_framebuffers.resize(getImageCount());
        for (size_t i = 0; i < getImageCount(); i++) {
            std::array<VkImageView, 2> attachments = {swapchain_image_views[i], depth_image_views[i]};

            VkExtent2D swapchain_extent = getSwapchainExtent();
            VkFramebufferCreateInfo framebuffer_info = {};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass = render_pass;
            framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebuffer_info.pAttachments = attachments.data();
            framebuffer_info.width = swapchain_extent.width;
            framebuffer_info.height = swapchain_extent.height;
            framebuffer_info.layers = 1;

            if (vkCreateFramebuffer(device.getDevice(), &framebuffer_info, nullptr, &swapchain_framebuffers[i]) != VK_SUCCESS) {
                spdlog::error("Failed to create framebuffer");
                exit(exitcode::FAILURE);
            }
        }
    }

    void Swapchain::createSyncObjects() {
        image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
        images_in_flight.resize(getImageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphore_info = {};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fence_info = {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            bool semaphore_result_a = vkCreateSemaphore(device.getDevice(), &semaphore_info, nullptr, &image_available_semaphores[i]) != VK_SUCCESS;
            bool semaphore_result_b = vkCreateSemaphore(device.getDevice(), &semaphore_info, nullptr, &render_finished_semaphores[i]) != VK_SUCCESS;
            bool fence_result = vkCreateFence(device.getDevice(), &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS;
            if (semaphore_result_a || semaphore_result_b || fence_result) {
                spdlog::error("Failed to create synchronisation objects for a frame");
                exit(exitcode::FAILURE);
            }
        }
    }

    VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &available_formats) {
        for (const auto &available_format : available_formats) {
            bool correct_format = available_format.format == VK_FORMAT_B8G8R8A8_SRGB;
            bool correct_colour_space = available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            if (correct_format && correct_colour_space) {
                return available_format;
            }
        }

        return available_formats[0];
    }

    VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &available_present_modes) {
        // for (const auto &available_present_mode : available_present_modes) {
        //     if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
        //         spdlog::debug("Present mode: Mailbox");
        //         return available_present_mode;
        //     }
        // }

        spdlog::debug("Present mode: V-Sync");
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            VkExtent2D actual_extent = window_extent;
            actual_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actual_extent.width));
            actual_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actual_extent.height));

            return actual_extent;
        }
    }

}
