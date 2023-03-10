#include "ComputeDevice.hpp"

// #define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace NEngine {
	void ComputeDevice::allocateCommandBuffers( 
			  VkDevice device, VkCommandPool command_pool
			, std::vector<VkCommandBuffer> & command_buffers, uint32_t command_buffer_count
	) {

		command_buffers.resize(command_buffer_count);

		VkCommandBufferAllocateInfo alloc_info{
			  .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO
			, .commandPool = command_pool
			, .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY
			, .commandBufferCount = command_buffer_count
		};

		if(vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data()) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate command buffer!");
	}

	void ComputeDevice::loadImage(
			  VkDevice device, VkPhysicalDevice physical_device, const VkCommandPool & command_pool, VkQueue queue
			, VkFormat format, const void * data, VkDeviceSize size_bytes, uint32_t width, uint32_t height
			, VkImage & image, VkDeviceMemory & image_memory, VkImageView & image_view
	) {

		VkBuffer temp_buffer;
		VkDeviceMemory temp_buffer_memory;
		createBuffer(device, physical_device, size_bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT
				, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				, temp_buffer, temp_buffer_memory);

		void* tmp_data;
		vkMapMemory(device, temp_buffer_memory, 0, size_bytes, 0, &tmp_data);
		memcpy(tmp_data, data, static_cast<size_t>(size_bytes));
		vkUnmapMemory(device, temp_buffer_memory);

		ComputeDevice::createImage(device, physical_device, width, height
			, format, VK_IMAGE_TILING_OPTIMAL
			, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
			, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			, image, image_memory);

		ComputeDevice::transitionImageLayout(device, command_pool, queue, image, format
				, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		ComputeDevice::copyBufferToImage(device, command_pool, queue, temp_buffer, image, width, height);
		ComputeDevice::transitionImageLayout(device, command_pool, queue, image, format
				, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device, temp_buffer, nullptr);
		vkFreeMemory(device, temp_buffer_memory, nullptr);

		image_view = ComputeDevice::createImageView(device, image, format, VK_IMAGE_ASPECT_COLOR_BIT);

		std::cout << "Image Loaded" << std::endl;
	}
}
