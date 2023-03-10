#pragma once

#include <vulkan/vulkan_core.h>

namespace NEngine {

struct DeviceVertexBuffer {
	VkBuffer buffer;
	VkDeviceMemory memory;
	//TODO destructor (we wait to rust)
		// vkDestroyBuffer(device->logical_device, index_buffer, nullptr);
		// vkFreeMemory(device->logical_device, index_buffer_memory, nullptr);
		// vkDestroyBuffer(device->logical_device, vertex_buffer, nullptr);
		// vkFreeMemory(device->logical_device, vertex_buffer_memory, nullptr);
};

}
