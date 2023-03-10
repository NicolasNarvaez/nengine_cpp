#pragma once
#include "ComputeDevice.hpp"
#include <vulkan/vulkan_core.h>

#include <algorithm>

namespace NEngine {

struct SwapChain {
	const ComputeDevice * device;
	VkSwapchainKHR swap_chain;

	VkExtent2D extent;
	VkFormat swap_chain_image_format;
	VkFormat depth_format;
	std::vector<VkImage> swap_chain_images;
	std::vector<VkImageView> swap_chain_image_views;
	VkImage depth_image;
	VkDeviceMemory depth_image_memory;
	VkImageView depth_image_view;

	enum Result {
		  SUBOPTIMAL = -1
		, SUCCESS = 0
		, ERROR
		, ERROR_OUTDATED // requires recreation
	};

	~SwapChain() {
		this->clean();
	};

	void clean() {
		for(size_t i = 0; i < swap_chain_image_views.size(); i++) {
			vkDestroyImageView(device->logical_device, swap_chain_image_views[i], nullptr);
		}
		vkDestroyImageView(device->logical_device, depth_image_view, nullptr);
		vkDestroyImage(device->logical_device, depth_image, nullptr);
		vkFreeMemory(device->logical_device, depth_image_memory, nullptr);

		vkDestroySwapchainKHR(device->logical_device, swap_chain, nullptr);

		std::cout << "deleted Swapchain" << std::endl;
	}

	Result acquireNextImage(uint32_t * image_index, VkSemaphore available_semaphore) {

		VkResult result = vkAcquireNextImageKHR(device->logical_device, swap_chain, UINT64_MAX
				, available_semaphore, VK_NULL_HANDLE, image_index);

		if(result == VK_ERROR_OUT_OF_DATE_KHR) return Result::ERROR_OUTDATED;
		if(result == VK_SUBOPTIMAL_KHR) return Result::SUBOPTIMAL;
		if(result == VK_SUCCESS) return Result::SUCCESS;
		else return Result::ERROR;
	}

	Result presentImage(uint32_t * image_index, VkSemaphore wait_sempahore) {

		VkSemaphore wait_sempahores[] {wait_sempahore};
		VkPresentInfoKHR present_info {
			  .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR
			, .waitSemaphoreCount = 1
			, .pWaitSemaphores = wait_sempahores
			, .swapchainCount = 1
			, .pSwapchains = &swap_chain
			, .pImageIndices = image_index
			, .pResults = nullptr // optional
		};
		VkResult result = vkQueuePresentKHR(device->present_queue, &present_info);

		if(result == VK_ERROR_OUT_OF_DATE_KHR) return Result::ERROR_OUTDATED;
		if(result == VK_SUBOPTIMAL_KHR) return Result::SUBOPTIMAL;
		if(result == VK_SUCCESS) return Result::SUCCESS;
		else return Result::ERROR;
	}

	/*
	 * SwapChain::
	 * Creates a swapchain and basic handlers
	 * @sideeffect swapChain, swap_chain_images, swap_chain_image_format, swap_chain_extent
	 */
	SwapChain(
			const ComputeDevice * device
	) : device{device} {
		std::cout << "swap_chain_image_views.size()" << this->swap_chain_image_views.size() << std::endl;
		createSwapChain(device->physical_device, device->logical_device, device->command_pool, device->graphics_queue
				, device->graphics_family, device->present_family
				, device->window, device->surface
				, swap_chain, swap_chain_images, swap_chain_image_views, extent, swap_chain_image_format
				, depth_image, depth_image_memory, depth_image_view
				, depth_format);
	}
	static void createSwapChain(
			  VkPhysicalDevice physical_device, VkDevice device, VkCommandPool command_pool, VkQueue graphics_queue
			, uint32_t graphics_family, uint32_t present_family
			, GLFWwindow * window, VkSurfaceKHR surface
			, VkSwapchainKHR & swap_chain
			, std::vector<VkImage> & swap_chain_images, std::vector<VkImageView> & swap_chain_image_views
			, VkExtent2D & extent, VkFormat & swap_chain_image_format
			, VkImage & depth_image, VkDeviceMemory & depth_image_memory, VkImageView & depth_image_view
			, VkFormat & depth_format
	) {
		auto swap_chain_support = ComputeDevice::getSwapChainSupportDetails(physical_device, surface);

		VkSurfaceFormatKHR surface_format = getSwapChainSurfaceFormat(swap_chain_support.formats);
		swap_chain_image_format = surface_format.format; // member assignment
		VkPresentModeKHR present_mode = getSwapChainPresentMode(swap_chain_support.presentModes);
		ComputeDevice::getWindowFramebufferSize(window, extent);

		uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;

#ifdef DEBUG
		std::cout << "swap_chain_support.capabilities.minImageCount" << swap_chain_support.capabilities.minImageCount << std::endl;
		std::cout << "swap_chain_support.capabilities.maxImageCount" << swap_chain_support.capabilities.maxImageCount<< std::endl;
#endif

		if(swap_chain_support.capabilities.maxImageCount > 0 && image_count < swap_chain_support.capabilities.maxImageCount)
			image_count = swap_chain_support.capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR create_info {
			  .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR
			, .surface = surface
			, .minImageCount = image_count
			, .imageFormat = surface_format.format
			, .imageColorSpace = surface_format.colorSpace
			, .imageExtent = extent
			, .imageArrayLayers = 1
			, .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT

			, .preTransform = swap_chain_support.capabilities.currentTransform
			, .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
			, .presentMode = present_mode
			, .clipped = VK_TRUE
			, .oldSwapchain = VK_NULL_HANDLE
		};
		uint32_t queue_family_indices[] = {
			graphics_family, present_family
		};
		if(graphics_family != present_family) {
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
		} else {
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0; // optional
			create_info.pQueueFamilyIndices = nullptr; // optional
		}
		// create swapchain
		if(vkCreateSwapchainKHR(device, &create_info, nullptr, &swap_chain) != VK_SUCCESS)
			throw std::runtime_error("failed to create swap chain!");
		std::cout << "Created SwapChain!" << std::endl;

		// create swapchain images
		// only minimum was specified, we retrieve final number
		vkGetSwapchainImagesKHR(device, swap_chain, &image_count, nullptr);
		swap_chain_images.resize(image_count);
		vkGetSwapchainImagesKHR(device, swap_chain, &image_count, swap_chain_images.data());

		swap_chain_image_views.resize(swap_chain_images.size());
		for(size_t i = 0; i < swap_chain_image_views.size(); i++)
			swap_chain_image_views[i] = ComputeDevice::createImageView( device
					, swap_chain_images[i], swap_chain_image_format , VK_IMAGE_ASPECT_COLOR_BIT);
		std::cout << "Created Images & Views" << std::endl;

		depth_format = ComputeDevice::findSupportedFormat(physical_device
				, {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}
				, VK_IMAGE_TILING_OPTIMAL
				, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
			);

		ComputeDevice::createImage(device, physical_device, extent.width, extent.height, depth_format
				, VK_IMAGE_TILING_OPTIMAL
				, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
				, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
				, depth_image, depth_image_memory
			);
		depth_image_view = ComputeDevice::createImageView(device, depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
		ComputeDevice::transitionImageLayout(device, command_pool, graphics_queue
				, depth_image, depth_format
				, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	// :ENT:SwapChain
	static VkSurfaceFormatKHR getSwapChainSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR> &formats
	) {
		for (const auto& format : formats)
			if(format.format == VK_FORMAT_B8G8R8A8_SRGB 
				&& format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
			)
				return format;

		return formats[0];
	}

	// :ENT:SwapChain
	static VkPresentModeKHR getSwapChainPresentMode(
			const std::vector<VkPresentModeKHR>& available_modes
	) {
		for(const auto& mode : available_modes)
			if(mode == VK_PRESENT_MODE_MAILBOX_KHR)
				return mode;

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	// :ENT:SwapChain
	// Deps: glfwGetFramebufferSize()
	static VkExtent2D truncateExtentSurfaceCapability(
		const VkSurfaceCapabilitiesKHR& surface_capability, const VkExtent2D & extent
	) {

		if(surface_capability.currentExtent.width != UINT32_MAX)
			return surface_capability.currentExtent;

		VkExtent2D clamped_extent {
			  .width = extent.width
			, .height = extent.height
		};

		clamped_extent.width = std::clamp(
				extent.width, surface_capability.minImageExtent.width
				, surface_capability.maxImageExtent.width);
		clamped_extent.height = std::clamp(
				extent.height, surface_capability.minImageExtent.height
				, surface_capability.maxImageExtent.height);

		return clamped_extent;
	}
};

}
