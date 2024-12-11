#include "engine/vulkan/swapchain.hpp"

#include <vulkan/vulkan.hpp>

#include "utils/exitcode.hpp"


namespace muon {

    Swapchain::Swapchain(Device &device, vk::Extent2D window_extent) : device{device}, window_extent{window_extent} {
        init();
    }

    Swapchain::Swapchain(Device &device, vk::Extent2D window_extent, std::shared_ptr<Swapchain> previous) : device{device}, window_extent{window_extent}, old_swapchain{previous} {
        init();
        old_swapchain = nullptr;
    }

    Swapchain::~Swapchain() {
        for (auto image_view : swapchain_image_views) {
            device.getDevice().destroyImageView(image_view, nullptr);
        }
        swapchain_image_views.clear();

        if (swapchain != nullptr) {
            device.getDevice().destroySwapchainKHR(swapchain, nullptr);
            swapchain = nullptr;
        }

        for (int i = 0; i < depth_images.size(); i++) {
            device.getDevice().destroyImageView(depth_image_views[i], nullptr);
            device.getDevice().destroyImage(depth_images[i], nullptr);
            device.getDevice().freeMemory(depth_image_memories[i], nullptr);
        }

        for (auto framebuffer : swapchain_framebuffers) {
            device.getDevice().destroyFramebuffer(framebuffer, nullptr);
        }

        device.getDevice().destroyRenderPass(render_pass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            device.getDevice().destroySemaphore(render_finished_semaphores[i], nullptr);
            device.getDevice().destroySemaphore(image_available_semaphores[i], nullptr);
            device.getDevice().destroyFence(in_flight_fences[i], nullptr);
        }
    }

    vk::Format Swapchain::findDepthFormat() {
        auto candidates = {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint};
        auto tiling = vk::ImageTiling::eOptimal;
        auto features = vk::FormatFeatureFlagBits::eDepthStencilAttachment;

        return device.findSupportedFormat(candidates, tiling, features);
    }

    vk::Result Swapchain::acquireNextImage(uint32_t* image_index) {
        auto result = device.getDevice().waitForFences(1, &in_flight_fences[current_frame], vk::True, std::numeric_limits<uint64_t>::max());
        if (result != vk::Result::eSuccess) {
            spdlog::warn("Failed to wait for fences");
        }

        result = device.getDevice().acquireNextImageKHR(
            swapchain,
            std::numeric_limits<uint64_t>::max(),
            image_available_semaphores[current_frame],
            nullptr,
            image_index
        );

        return result;
    }

    vk::Result Swapchain::submitCommandBuffers(const vk::CommandBuffer* buffers, uint32_t* image_index) {
        if (images_in_flight[*image_index] != nullptr) {
            auto result = device.getDevice().waitForFences(1, &images_in_flight[*image_index], vk::True, UINT64_MAX);
            if (result != vk::Result::eSuccess) {
                spdlog::warn("Failed to wait for fences");
            }
        }
        images_in_flight[*image_index] = in_flight_fences[current_frame];

        vk::SubmitInfo submitInfo = {};
        submitInfo.sType = vk::StructureType::eSubmitInfo;

        vk::Semaphore wait_semaphores[] = {image_available_semaphores[current_frame]};
        vk::PipelineStageFlags wait_stages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = wait_semaphores;
        submitInfo.pWaitDstStageMask = wait_stages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        vk::Semaphore signal_semaphores[] = {render_finished_semaphores[current_frame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signal_semaphores;

        auto result = device.getDevice().resetFences(1, &in_flight_fences[current_frame]);
        if (result != vk::Result::eSuccess) {
            spdlog::warn("Failed to reset fences");
        }
        result = device.getGraphicsQueue().submit(1, &submitInfo, in_flight_fences[current_frame]);
        if (result != vk::Result::eSuccess) {
            spdlog::error("Failed to submit draw command buffer");
            exit(exitcode::FAILURE);
        }

        vk::PresentInfoKHR present_info = {};
        present_info.sType = vk::StructureType::ePresentInfoKHR;

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = signal_semaphores;

        vk::SwapchainKHR swapchains[] = {swapchain};
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swapchains;

        present_info.pImageIndices = image_index;

        result = device.getPresentQueue().presentKHR(&present_info);
        if (result != vk::Result::eSuccess) {
            spdlog::warn("Failed to present swapchain");
        }

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
        SwapchainSupportDetails swapchain_support = device.getSwapchainSupport();

        vk::SurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(swapchain_support.formats);
        vk::PresentModeKHR present_mode = chooseSwapPresentMode(swapchain_support.present_modes);
        vk::Extent2D extent = chooseSwapExtent(swapchain_support.capabilities);

        uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
        if (swapchain_support.capabilities.maxImageCount > 0 &&
            image_count > swapchain_support.capabilities.maxImageCount) {
            image_count = swapchain_support.capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR create_info = {};
        create_info.sType = vk::StructureType::eSwapchainCreateInfoKHR;
        create_info.surface = device.getSurface();

        create_info.minImageCount = image_count;
        create_info.imageFormat = surface_format.format;
        create_info.imageColorSpace = surface_format.colorSpace;
        create_info.imageExtent = extent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

        QueueFamilyIndices indices = device.getPhysicalQueueFamilies();
        uint32_t queue_family_indices[] = {indices.graphics_family, indices.present_family};

        if (indices.graphics_family != indices.present_family) {
            create_info.imageSharingMode = vk::SharingMode::eConcurrent;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = queue_family_indices;
        } else {
            create_info.imageSharingMode = vk::SharingMode::eExclusive;
            create_info.queueFamilyIndexCount = 0;
            create_info.pQueueFamilyIndices = nullptr;
        }

        create_info.preTransform = swapchain_support.capabilities.currentTransform;
        create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

        create_info.presentMode = present_mode;
        create_info.clipped = vk::True;

        create_info.oldSwapchain = old_swapchain == nullptr ? nullptr : old_swapchain->swapchain;

        if (device.getDevice().createSwapchainKHR(&create_info, nullptr, &swapchain) != vk::Result::eSuccess) {
            spdlog::error("Failed to create swapchain");
            exit(exitcode::FAILURE);
        }

        auto result = device.getDevice().getSwapchainImagesKHR(swapchain, &image_count, nullptr);
        if (result != vk::Result::eSuccess) {
            spdlog::warn("Failed to get swapchain images");
        }
        swapchain_images.resize(image_count);
        result = device.getDevice().getSwapchainImagesKHR(swapchain, &image_count, swapchain_images.data());
        if (result != vk::Result::eSuccess) {
            spdlog::warn("Failed to get swapchain images");
        }

        swapchain_image_format = surface_format.format;
        swapchain_extent = extent;
    }

    void Swapchain::createImageViews() {
        swapchain_image_views.resize(swapchain_images.size());
        for (size_t i = 0; i < swapchain_images.size(); i++) {
            vk::ImageViewCreateInfo view_info{};
            view_info.sType = vk::StructureType::eImageViewCreateInfo;
            view_info.image = swapchain_images[i];
            view_info.viewType = vk::ImageViewType::e2D;
            view_info.format = swapchain_image_format;
            view_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.baseArrayLayer = 0;
            view_info.subresourceRange.layerCount = 1;

            if (device.getDevice().createImageView(&view_info, nullptr, &swapchain_image_views[i]) != vk::Result::eSuccess) {
                spdlog::error("Failed to create texture image view");
                exit(exitcode::FAILURE);
            }
        }
    }

    void Swapchain::createDepthResources() {
        vk::Format depth_format = findDepthFormat();
        swapchain_depth_format = depth_format;
        vk::Extent2D swapchain_extent = getSwapchainExtent();

        depth_images.resize(getImageCount());
        depth_image_memories.resize(getImageCount());
        depth_image_views.resize(getImageCount());

        for (int i = 0; i < depth_images.size(); i++) {
            vk::ImageCreateInfo image_info{};
            image_info.sType = vk::StructureType::eImageCreateInfo;
            image_info.imageType = vk::ImageType::e2D;
            image_info.extent.width = swapchain_extent.width;
            image_info.extent.height = swapchain_extent.height;
            image_info.extent.depth = 1;
            image_info.mipLevels = 1;
            image_info.arrayLayers = 1;
            image_info.format = depth_format;
            image_info.tiling = vk::ImageTiling::eOptimal;
            image_info.initialLayout = vk::ImageLayout::eUndefined;
            image_info.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
            image_info.samples = vk::SampleCountFlagBits::e1;
            image_info.sharingMode = vk::SharingMode::eExclusive;
            image_info.flags = vk::ImageCreateFlags{};

            device.createImageWithInfo(image_info, vk::MemoryPropertyFlagBits::eDeviceLocal, depth_images[i], depth_image_memories[i]);

            vk::ImageViewCreateInfo view_info{};
            view_info.sType = vk::StructureType::eImageViewCreateInfo;
            view_info.image = depth_images[i];
            view_info.viewType = vk::ImageViewType::e2D;
            view_info.format = depth_format;
            view_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.baseArrayLayer = 0;
            view_info.subresourceRange.layerCount = 1;

            if (device.getDevice().createImageView(&view_info, nullptr, &depth_image_views[i]) != vk::Result::eSuccess) {
                spdlog::error("Failed to create texture image view");
                exit(exitcode::FAILURE);
            }
        }
    }

    void Swapchain::createRenderPass() {
        vk::AttachmentDescription depth_attachment{};
        depth_attachment.format = findDepthFormat();
        depth_attachment.samples = vk::SampleCountFlagBits::e1;
        depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
        depth_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
        depth_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        depth_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        depth_attachment.initialLayout = vk::ImageLayout::eUndefined;
        depth_attachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::AttachmentReference depth_attachment_ref{};
        depth_attachment_ref.attachment = 1;
        depth_attachment_ref.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::AttachmentDescription colour_attachment = {};
        colour_attachment.format = getSwapchainImageFormat();
        colour_attachment.samples = vk::SampleCountFlagBits::e1;
        colour_attachment.loadOp = vk::AttachmentLoadOp::eClear;
        colour_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
        colour_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colour_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colour_attachment.initialLayout = vk::ImageLayout::eUndefined;
        colour_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference colour_attachement_ref = {};
        colour_attachement_ref.attachment = 0;
        colour_attachement_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass = {};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colour_attachement_ref;
        subpass.pDepthStencilAttachment = &depth_attachment_ref;

        vk::SubpassDependency dependency = {};
        dependency.dstSubpass = 0;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
        dependency.srcSubpass = vk::SubpassExternal;
        dependency.srcAccessMask = vk::AccessFlags{};
        dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;

        std::array<vk::AttachmentDescription, 2> attachments = {colour_attachment, depth_attachment};
        vk::RenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = vk::StructureType::eRenderPassCreateInfo;
        render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_info.pAttachments = attachments.data();
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &dependency;

        if (device.getDevice().createRenderPass(&render_pass_info, nullptr, &render_pass) != vk::Result::eSuccess) {
            spdlog::error("Failed to create render pass");
            exit(exitcode::FAILURE);
        }
    }

    void Swapchain::createFramebuffers() {
        swapchain_framebuffers.resize(getImageCount());
        for (size_t i = 0; i < getImageCount(); i++) {
            std::array<vk::ImageView, 2> attachments = {swapchain_image_views[i], depth_image_views[i]};

            vk::Extent2D swapchain_extent = getSwapchainExtent();
            vk::FramebufferCreateInfo framebuffer_info = {};
            framebuffer_info.sType = vk::StructureType::eFramebufferCreateInfo;
            framebuffer_info.renderPass = render_pass;
            framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebuffer_info.pAttachments = attachments.data();
            framebuffer_info.width = swapchain_extent.width;
            framebuffer_info.height = swapchain_extent.height;
            framebuffer_info.layers = 1;

            if (device.getDevice().createFramebuffer(&framebuffer_info, nullptr, &swapchain_framebuffers[i]) != vk::Result::eSuccess) {
                spdlog::error("Failed to create framebuffer");
                exit(exitcode::FAILURE);
            }
        }
    }

    void Swapchain::createSyncObjects() {
        image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
        images_in_flight.resize(getImageCount(), nullptr);

        vk::SemaphoreCreateInfo semaphore_info = {};
        semaphore_info.sType = vk::StructureType::eSemaphoreCreateInfo;

        vk::FenceCreateInfo fence_info = {};
        fence_info.sType = vk::StructureType::eFenceCreateInfo;
        fence_info.flags = vk::FenceCreateFlagBits::eSignaled;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            bool semaphore_result_a = device.getDevice().createSemaphore(&semaphore_info, nullptr, &image_available_semaphores[i]) != vk::Result::eSuccess;
            bool semaphore_result_b = device.getDevice().createSemaphore(&semaphore_info, nullptr, &render_finished_semaphores[i]) != vk::Result::eSuccess;
            bool fence_result = device.getDevice().createFence(&fence_info, nullptr, &in_flight_fences[i]) != vk::Result::eSuccess;
            if (semaphore_result_a || semaphore_result_b || fence_result) {
                spdlog::error("Failed to create synchronisation objects for a frame");
                exit(exitcode::FAILURE);
            }
        }
    }

    vk::SurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &available_formats) {
        for (const auto &available_format : available_formats) {
            bool correct_format = available_format.format == vk::Format::eB8G8R8A8Srgb;
            bool correct_colour_space = available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
            if (correct_format && correct_colour_space) {
                return available_format;
            }
        }

        return available_formats[0];
    }

    vk::PresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &available_present_modes) {
        // for (const auto &available_present_mode : available_present_modes) {
        //     if (available_present_mode == vk::PresentModeKHR::eMailbox) {
        //         spdlog::debug("Present mode: Mailbox");
        //         return available_present_mode;
        //     }
        // }

        spdlog::debug("Present mode: V-Sync");
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D Swapchain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            vk::Extent2D actual_extent = window_extent;
            actual_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actual_extent.width));
            actual_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actual_extent.height));

            return actual_extent;
        }
    }

}
