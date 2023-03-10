#pragma once

#include "../../util.hpp"

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstring>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

namespace NEngine {
struct APIInstance {
	VkInstance instance;
};

struct ComputeDevice {
	VkPhysicalDevice physical_device;
	const std::vector<const char*> & validation_layers; // logical_device
	const std::vector<const char*> & device_extensions; // logical_device
	VkPhysicalDeviceProperties properties;
	VkDevice logical_device;
	VkCommandPool command_pool;

	uint32_t graphics_family;
	uint32_t present_family;

	VkQueue present_queue;
	VkQueue graphics_queue;

	// resources
	VkSurfaceKHR surface;
	GLFWwindow * window;



	// ComputeDevice::
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphics_family;
		std::optional<uint32_t> present_family;
		bool isComplete() {
				return graphics_family.has_value() && present_family.has_value();
		}
	};

	// ComputeDevice::
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	ComputeDevice(
			  VkInstance instance, GLFWwindow * window
			, const std::vector<const char*> & validation_layers, const std::vector<const char*> & device_extensions
	) : window{window}, validation_layers{validation_layers}, device_extensions{device_extensions} {

		createSurface(instance, window, surface);
		pickPhysicalDevice(instance, surface, device_extensions, physical_device);

		createLogicalDevice(physical_device, surface, validation_layers, device_extensions
				, logical_device, graphics_queue, present_queue);

		vkGetPhysicalDeviceProperties(physical_device, &properties);

		printVersion(physical_device);

		auto queue_families = ComputeDevice::findQueueFamilies(physical_device, surface);

		graphics_family = queue_families.graphics_family.value();
		present_family = queue_families.present_family.value();
	}

	/*
	 * ComputeDevice::constructor()
	 * @sideeffect device, graphics_queue, present_queue
	 */
	static void createLogicalDevice(
			  VkPhysicalDevice physical_device, const VkSurfaceKHR & surface
			, const std::vector<const char*> & validation_layers, const std::vector<const char*> & device_extensions
			, VkDevice & device, VkQueue & graphics_queue, VkQueue & present_queue
	) {

		bool enable_validation_layers = validation_layers.size() != 0;

		auto indices = ComputeDevice::findQueueFamilies(physical_device, surface);
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		std::set<uint32_t> queue_families = {
			indices.graphics_family.value(), indices.present_family.value()
		};
		float queue_priorities = 1.0f;
		for(uint32_t family : queue_families)
			queue_create_infos.push_back({
				  .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO
				, .queueFamilyIndex = family
				, .queueCount = 1
				, .pQueuePriorities = &queue_priorities
			});
		VkPhysicalDeviceFeatures device_features {
			.samplerAnisotropy = VK_TRUE
		};
		VkDeviceCreateInfo create_info {
			  .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO
			, .queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size())
			, .pQueueCreateInfos = queue_create_infos.data()
			, .enabledLayerCount = enable_validation_layers? static_cast<uint32_t>(validation_layers.size()) : 0
			, .ppEnabledLayerNames = enable_validation_layers? validation_layers.data() : nullptr
			, .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size())
			, .ppEnabledExtensionNames = device_extensions.data()
			, .pEnabledFeatures = &device_features
			// extensions, validations, same as instance (deprecate this)
		};

		if(vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS)
			throw std::runtime_error("failed to create the logical device");

		vkGetDeviceQueue(device, indices.graphics_family.value(), 0, &graphics_queue);
		vkGetDeviceQueue(device, indices.present_family.value(), 0, &present_queue);
		
		std::cout << "Created Logical Device" << std::endl;
	}

	/*
	 * APIInstance::
	 * @sideeffect surface
	 */
	static void createSurface(VkInstance instance, GLFWwindow * window, VkSurfaceKHR & surface) {
		if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("failed to create window surface");
	}

	static void getWindowFramebufferSize(GLFWwindow * window, VkExtent2D & extent) {

		std::cout << "inside getWindow"  << std::endl;
		int width = 0, height = 0;

		glfwGetFramebufferSize(window, &width, &height);
		while(width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		extent.width = static_cast<uint32_t>(width);
		extent.height = static_cast<uint32_t>(height);
	}

	// ComputeDevice::
	static QueueFamilyIndices findQueueFamilies(
		VkPhysicalDevice physical_device, VkSurfaceKHR surface
	) {
		QueueFamilyIndices indices;

		uint32_t queue_family_n {0};
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_n, nullptr);
		std::vector<VkQueueFamilyProperties> queue_families(queue_family_n);
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_n, queue_families.data());

		int family_index {0};
		for(const auto& family : queue_families) {
			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, family_index, surface, &present_support);

			if(present_support) indices.present_family = family_index;
			if(family.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphics_family = family_index;

			if(indices.isComplete()) break;
			family_index++;
		}

		bool completed = indices.isComplete();


		return indices;
	}

	/*
	 * ComputeDevice::
	 * @sideeffect physical_device
	 */
	static void pickPhysicalDevice(
			  VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*> & device_extensions
			,  VkPhysicalDevice & physical_device
	) {

		uint32_t deviceCount {0};
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if(deviceCount == 0) throw std::runtime_error("failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for(const auto& device: devices)
				if(isDeviceSuitable(device, surface, device_extensions)) {
						physical_device = device;
						break;
				}
		if(physical_device == VK_NULL_HANDLE) throw std::runtime_error("failed to find a suitable GPU");

		std::cout << "Found adequeate Device!" << std::endl;
	}

	// ComputeDevice::
	// Checks for extension, swapchain and features
	static bool isDeviceSuitable(
		VkPhysicalDevice physical_device, VkSurfaceKHR surface, const std::vector<const char *> extensions
	) {
		QueueFamilyIndices indices = findQueueFamilies(physical_device, surface);

		bool supports_extensions = checkDeviceExtensionSupport(physical_device, extensions);

		bool supports_swapchain = false;
		if(supports_extensions) {
			SwapChainSupportDetails swap_chain_support = getSwapChainSupportDetails(physical_device, surface);
			supports_swapchain = 
					!swap_chain_support.formats.empty() 
				&& !swap_chain_support.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supported_features;
		vkGetPhysicalDeviceFeatures(physical_device, &supported_features);
		bool supports_features = supported_features.samplerAnisotropy;

		return indices.isComplete() 
			&& supports_extensions 
			&& supports_swapchain 
			&& supports_features;
	}

	// ComputeDevice::
	static SwapChainSupportDetails getSwapChainSupportDetails(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
				physical_device, surface, &details.capabilities);

		uint32_t format_n;
		vkGetPhysicalDeviceSurfaceFormatsKHR(
				physical_device, surface, &format_n, nullptr);
		if(format_n != 0) {
			details.formats.resize(format_n);
			vkGetPhysicalDeviceSurfaceFormatsKHR(
					physical_device, surface, &format_n, details.formats.data());
		}

		uint32_t present_mode_n;
		vkGetPhysicalDeviceSurfacePresentModesKHR(
				physical_device, surface, &present_mode_n, nullptr);
		if(format_n != 0) {
			details.presentModes.resize(present_mode_n);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface
					, &present_mode_n, details.presentModes.data()
			);
		}

		return details;
	}

	// ComputeDevice::
	// Deps: vkEnumerateDeviceExtensionProperties
	static bool checkDeviceExtensionSupport(
		VkPhysicalDevice device, const std::vector<const char*> &required_extensions
	) {
		uint32_t extension_count;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

		std::vector<VkExtensionProperties> available_extensions(extension_count);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

		std::set<std::string> unsatisfied(required_extensions.begin(), required_extensions.end());

		for(const auto& extension : available_extensions) {
			unsatisfied.erase(extension.extensionName);
		}

		return unsatisfied.empty();
	}

	// @sideeffect command_pool: created with vkCreateCommandPool
	void createCommandPool(uint32_t queue_family, VkCommandPool & command_pool) {
		return createCommandPool(logical_device, queue_family, command_pool);
	};
	static void createCommandPool(VkDevice logical_device, uint32_t queue_family, VkCommandPool & command_pool) {

		VkCommandPoolCreateInfo pool_info{
			  .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO
			, .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
			, .queueFamilyIndex = queue_family
		};
		if(vkCreateCommandPool(logical_device, &pool_info, nullptr, &command_pool) != VK_SUCCESS)
			throw std::runtime_error("failed to create command pool!");

		std::cout << "Created Command Pool" << std::endl;	
	}

	// Wrapper for vkAllocateCommandBuffers.
	void allocateCommandBuffers(std::vector<VkCommandBuffer> & command_buffers, uint32_t command_buffer_count) const {
		allocateCommandBuffers(logical_device, command_pool, command_buffers, command_buffer_count);
	};
	static void allocateCommandBuffers( 
			  VkDevice logical_device, VkCommandPool command_pool
			, std::vector<VkCommandBuffer> & command_buffers, uint32_t command_buffer_count
	);

	// Creates temp buffer, copies image data to buffer, then image, transitions image to transfer, then shader_read
	// , removes temp resources.
	// TODO: add stronger sincronization on distributed graphics queue
	// queue submision wont ensure execution order (only initiation order) on same queue
	//	link: https://vulkan-tutorial.com/Depth_buffering
	//		check comments
	//		extra: "Yet another blog explaining Vulkan synchronization"
	//	@sideeffect image, image_memory, image_view
	void loadImage(
			VkQueue queue
			, VkFormat format, const void * data, VkDeviceSize size_bytes, uint32_t width, uint32_t height
			, VkImage & image, VkDeviceMemory & image_memory, VkImageView & image_view
	) const {
		return loadImage(logical_device, physical_device, command_pool, queue
				, format, data, size_bytes, width, height
				, image, image_memory, image_view);
	};
	static void loadImage(
			  VkDevice logical_device, VkPhysicalDevice physical_device, const VkCommandPool & command_pool, VkQueue queue
			, VkFormat format, const void * data, VkDeviceSize size_bytes, uint32_t width, uint32_t height
			, VkImage & image, VkDeviceMemory & image_memory, VkImageView & image_view
	);

	// TODO: clean calls to vkGetPhysicalDeviceProperties
	// @sideeffect sampler: created with vkCreateSampler
	static void createTextureSamplerLinear(const ComputeDevice * device, VkSampler & sampler) {

		VkPhysicalDeviceProperties properties {};
		vkGetPhysicalDeviceProperties(device->physical_device, &properties);
		
		VkSamplerCreateInfo sampler_info {
			  .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO
			, .magFilter = VK_FILTER_LINEAR
			, .minFilter = VK_FILTER_LINEAR
			, .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR
			, .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT
			, .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT
			, .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT

			, .mipLodBias = 0.0f
			, .anisotropyEnable = VK_TRUE
			, .maxAnisotropy = properties.limits.maxSamplerAnisotropy

			, .compareEnable = VK_FALSE
			, .compareOp = VK_COMPARE_OP_ALWAYS

			, .minLod = 1.0f
			, .maxLod = 1.0f
			, .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK
			, .unnormalizedCoordinates = VK_FALSE
		};

		if(vkCreateSampler(device->logical_device, &sampler_info, nullptr, &sampler) != VK_SUCCESS)
			throw std::runtime_error("failed to create texture sampler!");
	}

	
	VkShaderModule createShaderModule(const std::vector<char>& code) {
		return createShaderModule(this->logical_device, code);
	};
	static VkShaderModule createShaderModule(
			VkDevice logical_device, const std::vector<char>& code
	) {

	VkShaderModuleCreateInfo create_info{
		  .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
		, .codeSize = code.size()
		, .pCode = reinterpret_cast<const uint32_t*>(code.data())
	};

	VkShaderModule shader_module;
	if(vkCreateShaderModule(logical_device, &create_info, nullptr, &shader_module) 
			!= VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shader_module;
}

	// 2D image
	static void createImage(
			  VkDevice logical_device, VkPhysicalDevice physical_device
			, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage
			, VkMemoryPropertyFlags memory_properties
			, VkImage& image, VkDeviceMemory& memory
	) {
		createImage(logical_device, physical_device
				, {.width = width, .height = height, .depth = 1}, VK_IMAGE_TYPE_2D, format, tiling, usage, memory_properties
				, image, memory);
	}
	
	// 3D image
	static void createImage(
			  VkDevice logical_device, VkPhysicalDevice physical_device
			, VkExtent3D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage
			, VkMemoryPropertyFlags memory_properties
			, VkImage& image, VkDeviceMemory& memory
	) {
		createImage(logical_device, physical_device
				, extent, VK_IMAGE_TYPE_3D, format, tiling, usage, memory_properties
				, image, memory);
	}

	// Creates image, memory & binds
	static void createImage(
			  VkDevice logical_device, VkPhysicalDevice physical_device
			, const VkExtent3D & extent, VkImageType image_type, VkFormat format
			, VkImageTiling tiling, VkImageUsageFlags usage
			, VkMemoryPropertyFlags memory_properties
			, VkImage& image, VkDeviceMemory& memory
	) {
		VkImageCreateInfo image_info {
			  .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO
			, .imageType = image_type
			, .format = format
			, .extent = extent
			, .mipLevels = 1
			, .arrayLayers = 1
			, .samples = VK_SAMPLE_COUNT_1_BIT
			, .tiling = tiling
			, .usage = usage
			, .sharingMode = VK_SHARING_MODE_EXCLUSIVE
			, .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};
		if(vkCreateImage(logical_device, &image_info, nullptr, &image) != VK_SUCCESS)
			throw std::runtime_error("failed to create Image!");

		VkMemoryRequirements memory_reqs;
		vkGetImageMemoryRequirements(logical_device, image, &memory_reqs);
		VkMemoryAllocateInfo alloc_info{
			  .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
			, .allocationSize = memory_reqs.size
			, .memoryTypeIndex = findMemoryType(
					physical_device, memory_properties, memory_reqs.memoryTypeBits)
		};
		if(vkAllocateMemory(logical_device, &alloc_info, nullptr, &memory) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate image memory!");

		vkBindImageMemory(logical_device, image, memory, 0);
		std::cout << "Created image" << std::endl;
	}

	// ComputeDevice::
	// Submits/awaits image transition correlating access masks and stages, sets aspectMask according to format.
	static void transitionImageLayout(
			  VkDevice logical_device, VkCommandPool command_pool, VkQueue queue
			, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout
	) {

		VkCommandBuffer command_buffer = beginSingleTimeCommands(logical_device, command_pool);

		VkPipelineStageFlags src_stage, dst_stage;
		VkAccessFlags src_access_mask, dst_access_mask;

		if(		   old_layout == VK_IMAGE_LAYOUT_UNDEFINED
				&& new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		) {
			src_access_mask = 0;
			dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;

			src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

		} else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL 
				&& new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		) {
			src_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dst_access_mask = VK_ACCESS_SHADER_READ_BIT;

			src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		} else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED 
				&& new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		) {
			src_access_mask = 0;
			dst_access_mask =	VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT 
									| VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		} else {
			throw std::runtime_error("unsupported layout transition!");
		}

		VkImageMemoryBarrier barrier {
			  .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER
			, .srcAccessMask = src_access_mask
			, .dstAccessMask = dst_access_mask
			, .oldLayout = old_layout
			, .newLayout = new_layout
			, .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED // disable family transition now
			, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED // disable family transition now
			, .image = image
			, .subresourceRange = {
				  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
				, .baseMipLevel = 0
				, .levelCount = 1
				, .baseArrayLayer = 0
				, .layerCount = 1
				}
		};

		if(new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if(format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) { 
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		} else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		vkCmdPipelineBarrier(
			  command_buffer
			, src_stage, dst_stage
			, 0
			, 0, nullptr
			, 0, nullptr
			, 1, &barrier
		);

		endSingleTimeCommands(logical_device, command_pool, command_buffer, queue);
	}

	// ComputeDevice::
	// Simplest image possible
	static VkImageView createImageView(
			VkDevice logical_device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags
	) {

		VkImageView image_view;

		VkImageViewCreateInfo view_info {
			  .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO
			, .image = image
			, .viewType = VK_IMAGE_VIEW_TYPE_2D
			, .format = format
			// view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // optional
			// view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY; // optional
			// view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY; // optional
			// view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY; // optional
			, .subresourceRange = {
				  .aspectMask = aspect_flags
				, .baseMipLevel = 0
				, .levelCount = 1
				, .baseArrayLayer = 0
				, .layerCount = 1
			}
		};
		if(vkCreateImageView(logical_device, &view_info, nullptr, &image_view) != VK_SUCCESS)
			throw std::runtime_error("failed to create image views!");

		return image_view;
	}

	/**
	 * ComputeDevice::
	 * Creates a device-local buffer from void* data_src.
	 * Uses "createBuffer()" and "copyBuffer()" to make a VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT 
	 * buffer from a VK_MEMORY_PROPERTY_HOST_[VISIBLE, COHERENT]_BIT staging buffer
	 * as a transfer source, then removes the staging buffer.
	 * @sideeffect buffer, buffer_memory have data_src written into
	 */
	void createDeviceBuffer(
			  VkQueue queue, const void* data_src, const VkDeviceSize &data_size
			, VkBuffer &buffer, VkDeviceMemory &buffer_memory, VkBufferUsageFlagBits usage_bits
	) const {
		return createDeviceBuffer(logical_device, physical_device, command_pool, queue, data_src, data_size, buffer, buffer_memory, usage_bits);
	};
	static void createDeviceBuffer(
			  const VkDevice &logical_device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue queue
			, const void* data_src, const VkDeviceSize &data_size
			, VkBuffer &buffer, VkDeviceMemory &buffer_memory, VkBufferUsageFlagBits usage_bits
	) {

		VkBuffer staging_buffer;
		VkDeviceMemory staging_buffer_memory;
		createBuffer(logical_device, physical_device, data_size
				, VK_BUFFER_USAGE_TRANSFER_SRC_BIT
				, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				, staging_buffer, staging_buffer_memory);

		void * data_mapping;
		vkMapMemory(logical_device, staging_buffer_memory, 0, data_size, 0, &data_mapping);
		memcpy(data_mapping, data_src, (size_t) data_size);
		vkUnmapMemory(logical_device, staging_buffer_memory);

		createBuffer(logical_device, physical_device, data_size
				, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage_bits
				, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
				, buffer, buffer_memory);

		copyBuffer(logical_device, command_pool, queue, staging_buffer, buffer, data_size);

		vkDestroyBuffer(logical_device, staging_buffer, nullptr);
		vkFreeMemory(logical_device, staging_buffer_memory, nullptr);
	}

	// ComputeDevice::
	// @sideeffect buffer_memory
	static void setDeviceMemory(
				VkDevice device, const void * src_data, uint32_t src_size, VkDeviceMemory buffer_memory, uint32_t offset = 0
	) {

		void* data;
		vkMapMemory(device, buffer_memory, offset, src_size, 0, &data);
		memcpy(data, src_data, src_size);
		vkUnmapMemory(device, buffer_memory);
	}

	static std::string deviceMemoryToString(VkDevice device, VkDeviceMemory memory, uint32_t size_bytes) {
		void* data;
		vkMapMemory(device, memory, 0, size_bytes, 0, &data);

		return util::stringBinary(data, size_bytes);
	}

	// ComputeDevice::
	// Creates a Buffer and its memory (binded).
	static void createBuffer(
			  const VkDevice &logical_device, const VkPhysicalDevice &physical_device
			, const VkDeviceSize &size, const VkBufferUsageFlags &usage
			, const VkMemoryPropertyFlags &memory_properties
			, VkBuffer &buffer, VkDeviceMemory &memory
	) {

		VkBufferCreateInfo buffer_info {
			  .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO
			, .size = size
			, .usage = usage
			, .sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};
		if(vkCreateBuffer(logical_device, &buffer_info, nullptr, &buffer) != VK_SUCCESS)
			throw std::runtime_error("failed to create buffer");

		VkMemoryRequirements memory_reqs;
		vkGetBufferMemoryRequirements(logical_device, buffer, &memory_reqs);
		VkMemoryAllocateInfo alloc_info {
			  .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
			, .allocationSize = memory_reqs.size
			, .memoryTypeIndex = findMemoryType(
					physical_device, memory_properties, memory_reqs.memoryTypeBits)
		};
		if(vkAllocateMemory(logical_device, &alloc_info, nullptr, &memory) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate buffer memory");

		vkBindBufferMemory(logical_device, buffer, memory, 0);
		std::cout << "buffer created" << std::endl;
	}

	// ComputeDevice::
	// vkCmdCopyBufferToImage ONE_TIME_SUBMIT wrapper
	static void copyBufferToImage(
			VkDevice logical_device, VkCommandPool command_pool, VkQueue queue
			, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height
	) {
		VkCommandBuffer command_buffer = beginSingleTimeCommands(logical_device, command_pool);

		VkBufferImageCopy region{
			  .bufferOffset = 0
			, .bufferRowLength = 0
			, .bufferImageHeight = 0
			, .imageSubresource = { 
				  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
				, .mipLevel = 0
				, .baseArrayLayer = 0
				, .layerCount = 1
			}
			, .imageOffset = {0, 0, 0}
			, .imageExtent = {width, height, 1}
		};
		vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		
		endSingleTimeCommands(logical_device, command_pool, command_buffer, queue);
	}

	// ComputeDevice::
	// vkCmdCopyBuffer with ONE_TIME_SUBMIT wrapper
	static void copyBuffer(
			VkDevice logical_device, VkCommandPool command_pool, VkQueue queue
			, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size
	) {
		VkCommandBuffer command_buffer = beginSingleTimeCommands(logical_device, command_pool);

		VkBufferCopy region{};
		// region.srcOffset = 0; // optional
		// region.dstOffset = 0; // optional
		region.size = size;
		vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &region);

		endSingleTimeCommands(logical_device, command_pool, command_buffer, queue);
	}
	//////////////////////////////////// Command Buffer Commands
	// ComputeDevice::
	// Creates command buffer (PRIMARY), begins recording (ONE_TIME_SUBMIT)
	static VkCommandBuffer beginSingleTimeCommands(VkDevice logical_device, VkCommandPool command_pool) {

		VkCommandBufferAllocateInfo alloc_info{
			  .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO
			, .commandPool = command_pool
			, .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY
			, .commandBufferCount = 1
		};
		VkCommandBuffer command_buffer;
		vkAllocateCommandBuffers(logical_device, &alloc_info, &command_buffer);

		VkCommandBufferBeginInfo beginInfo{
			  .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
			, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};
		vkBeginCommandBuffer(command_buffer, &beginInfo);

		return command_buffer;
	}

	// ComputeDevice::
	// Stops recording, submits to execution, vkQueueWaitIdle(), then frees the command buffer
	static void endSingleTimeCommands(
			VkDevice logical_device, VkCommandPool command_pool, VkCommandBuffer command_buffer, VkQueue queue
	) {
		vkEndCommandBuffer(command_buffer);

		VkSubmitInfo submit_info {
			  .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO
			, .commandBufferCount = 1
			, .pCommandBuffers = &command_buffer
		};
		vkQueueSubmit(queue, 1 , &submit_info, VK_NULL_HANDLE);

		vkQueueWaitIdle(queue);

		vkFreeCommandBuffers(logical_device, command_pool, 1, &command_buffer);
	}

	//////////////////////////////////// Buffers
	/**
	 * finds a VkMemoryType including type_filter and properties in device
	 * TODO: investigate using mem_props as argument (device wrapper?)
	 */
	static uint32_t findMemoryType(const VkPhysicalDevice &physical_device
			, const VkMemoryPropertyFlags &properties
			, const uint32_t &type_filter
	) {
		
		VkPhysicalDeviceMemoryProperties mem_props;
		vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

		for(uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
			if( (type_filter & (1 << i)) 
				&& (mem_props.memoryTypes[i].propertyFlags & properties) 
					== properties) {
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type!");
	}

	// ComputeDevice::
	// Retrieves format from physical_device that supports features and any of the candidates, for the given tiling
	static VkFormat findSupportedFormat(
			  VkPhysicalDevice physical_device
			, const std::vector<VkFormat> & candidates, VkImageTiling tiling, VkFormatFeatureFlags features
	) {
		for(VkFormat format : candidates) {
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(physical_device, format, &properties);

			if(tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
				return format;
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
				return format;
		}

		throw std::runtime_error("failed to find supported format!");
	}

	// ComputeDevice::
	// TODO:: Test device suitability selector, add user selection logic
	static int rateDevice(VkPhysicalDevice device) {
		int score = 0;
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceProperties(device, &properties);
		vkGetPhysicalDeviceFeatures(device, &features);

		if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				// && features.geometryShader;
				score += 1000;
		}

		// Maximum possible size of textures affects graphics quality
		score += properties.limits.maxImageDimension2D;

		if(!features.geometryShader) return 0;

		return score;
	}

	////////////////////////////// DEBUG
	// ComputeDevice::
	// Just prints the instance and device api versions
	static void printVersion(VkPhysicalDevice physical_device) {
		uint32_t instance_api_version;
		vkEnumerateInstanceVersion(&instance_api_version);

		VkPhysicalDeviceProperties device_properties{};
		vkGetPhysicalDeviceProperties(physical_device, &device_properties);

		uint32_t device_api_version = device_properties.apiVersion;
		
		std::cout << "Vulkan InstanceLevel Version: " << instance_api_version << std::endl;
		std::cout << "	  variant : " << VK_API_VERSION_VARIANT(instance_api_version) << std::endl;
		std::cout << "	  major   : " << VK_API_VERSION_MAJOR(instance_api_version) << std::endl;
		std::cout << "	  minor   : " << VK_API_VERSION_MINOR(instance_api_version) << std::endl;
		std::cout << "	  patch   : " << VK_API_VERSION_PATCH(instance_api_version) << std::endl;

		std::cout << "Vulkan DeviceLevel Version: " << device_api_version << std::endl;
		std::cout << "	  variant : " << VK_API_VERSION_VARIANT(device_api_version) << std::endl;
		std::cout << "	  major   : " << VK_API_VERSION_MAJOR(device_api_version) << std::endl;
		std::cout << "	  minor   : " << VK_API_VERSION_MINOR(device_api_version) << std::endl;
		std::cout << "	  patch   : " << VK_API_VERSION_PATCH(device_api_version) << std::endl;
	}
};
}
