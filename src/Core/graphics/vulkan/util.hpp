#pragma once
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <cstring>
#include <iostream>
#include <vector>

namespace NEngine {

/**
 * GraphicsDevice: logical, physical device instances, multiple device logic
 */

/**
 * VulkanLLRenderer: 
 */

// return true if all globally required_layers are supported
bool checkValidationLayerSupport(
		const std::vector<const char*> required_layers
);

VkResult createDebugUtilsMessengerEXT(
		  VkInstance instance
		, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo
		, const VkAllocationCallbacks* pAllocator
		, VkDebugUtilsMessengerEXT* pDebugMessenger) ;


void destroyDebugUtilsMessengerEXT(
		  VkInstance instance
		, VkDebugUtilsMessengerEXT debugMessenger
		, const VkAllocationCallbacks* pAllocator) ;

VKAPI_ATTR VkBool32 VKAPI_CALL defaultDebugUtilsMessengerCallback(
	  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity
	, VkDebugUtilsMessageTypeFlagsEXT messageType
	, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData
	, void* pUserData
		);

// Deps: debugCallback
void populateDebugMessengerCreateInfo(
		VkDebugUtilsMessengerCreateInfoEXT & createInfo);

}
