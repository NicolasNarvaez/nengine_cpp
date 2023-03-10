#pragma once

#include "../HostImage.hpp"

#include "ComputeDevice.hpp"

#include "resources.hpp"

#include <vulkan/vulkan_core.h>

namespace NEngine {

struct DeviceImage {
	const ComputeDevice * device;
	const HostImage * host_image;
	VkFormat format;

	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory;
	VkImageView view;
	VkSampler sampler;

	// sampler
	DeviceImage(const ComputeDevice * device, const HostImage * host_image) 
		: device{device}, host_image{host_image}, format{toVkFormat(host_image->format)} {
		load();
	};
	void load() {
		if(image != VK_NULL_HANDLE) return;
		device->loadImage(device->graphics_queue
				, format, host_image->data, (VkDeviceSize)host_image->size_bytes, host_image->width, host_image->height
				, image, memory, view);
		ComputeDevice::createTextureSamplerLinear(device, sampler);
	};
	void unload() {
		vkDestroySampler(device->logical_device, sampler, nullptr);
		vkDestroyImageView(device->logical_device, view, nullptr);
		vkDestroyImage(device->logical_device, image, nullptr);
		vkFreeMemory(device->logical_device, memory, nullptr);
	};
	// ~DeviceImage() {unload();};
	//
};

}
