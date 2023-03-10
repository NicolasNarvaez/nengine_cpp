#pragma once
#include "LLRenderer.hpp"

#include "graphics/vulkan/ComputeDevice.hpp"
#include "graphics/vulkan/SwapChain.hpp"
#include "graphics/vulkan/RenderPipeline.hpp"

#include "os/fs.hpp"
#include "graphics/resources.hpp"
#include "graphics/vulkan/util.hpp"

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// #define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <tiny_obj_loader.h>

#include <algorithm>
#include <array>
#include <bitset>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <iostream>
#include <fstream>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

namespace NEngine {

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 800;
const int MAX_FRAMES_IN_FLIGHT = 2;

// This is more a rendering context than a renderer
// Renderer context is equivalent to api version dependant, at an api level and instance arguments, holds all
// related resources and pipelines.
//
// Future implementation: APIInstance
// , LLGraphicsInstace{configures api_instance, owns contexts}
// , LLSurface{implicit window instance, surface}
// , LLGraphicsContext{renderers(render pipeline + context), rendering resources, threading, etc}
//
class VulkanLLRenderer : public LLRenderer {
#ifdef NDEBUG
	const bool enable_validation_layers = false;
#else
	const bool enable_validation_layers = true;
#endif
	const std::vector<const char*> validation_layers {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> device_extensions {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	// ------------------------------------------------------------------------
	// ------------------------- graphics context
	ComputeDevice * device;
	SwapChain * swap_chain;
	
	// 
	GLFWwindow * window;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkInstance instance;

	// ------------------------------------------------------------------------
	// ------------------------- rendering state, resources
	
	RenderPipeline * render_pipeline;
	
	// Synchronization: Rendering, presentation
	std::vector<VkSemaphore> image_available_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;
	std::vector<VkFence> in_flight_fences;
	uint32_t current_frame = 0;
	bool framebuffer_resized = false;

	// Scene and world buffer
	RenderScene * render_scene;
	BufferData<UniformBufferObject> * t_world_ubo;

	public:
	void run(RenderScene * src_scene) {
		// VertexData -> [multiplexed] vertex/color/texture, index data, + vertex descriptors
		// material -> shaders args, texture resources, descriptors
		// object -> uniform data
		// scene -> uniform data
		// scenegraph -> input bindings, descriptors

		/**
		 * Main interface to scene resources. 
		 * Contains scene resources, objects, descriptions and computed graphics_pipeline and
		 * renderpass using a renderer reference.
		 */
		render_scene = src_scene;

		t_world_ubo = new BufferData<UniformBufferObject>;
		render_scene->scene_buffer = new DataBuffer(t_world_ubo);

		initWindow();
		initVulkan();

		render_scene->viewport_extent = fromVkExtent(swap_chain->extent);

		mainLoop();
		cleanup();
	}

	private:

	// Deps: WIDTH, HEIGHT, "Vulkan", framebufferResizeCallback
	// Ups: window
	void initWindow() {
		glfwInit();

		int a =0;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		// glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	static void framebufferResizeCallback(GLFWwindow * window, int width, int height) {
		auto app = reinterpret_cast<VulkanLLRenderer*>(glfwGetWindowUserPointer(window));
		app->framebuffer_resized = true;
	}

	void initVulkan() {
		std::cout << "MAX_FRAMES_IN_FLIGHT" << MAX_FRAMES_IN_FLIGHT << std::endl;

		createInstance(validation_layers, instance);
		if(enable_validation_layers) setupDebugMessenger(instance, debug_messenger);

		device = new ComputeDevice(instance, window, validation_layers, device_extensions);

		device->createCommandPool(device->graphics_family, device->command_pool);

		swap_chain = new SwapChain(device);

		render_pipeline = new RenderPipeline(device, swap_chain, MAX_FRAMES_IN_FLIGHT, render_scene);

		// Renderer
		createSyncObjects(device->logical_device, MAX_FRAMES_IN_FLIGHT);
	}

	void cleanup() {
		render_pipeline->destroy();
		swap_chain->clean();

		// Resources::
		// Buffers:: uniform, vertex, index

		// Sync primitives
		for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device->logical_device, render_finished_semaphores[i], nullptr);
			vkDestroySemaphore(device->logical_device, image_available_semaphores[i], nullptr);
			vkDestroyFence(device->logical_device, in_flight_fences[i], nullptr);
		}

		vkDestroyCommandPool(device->logical_device, device->command_pool, nullptr);
		vkDestroyDevice(device->logical_device, nullptr);
		//////////// END device->logical_device context

		if(enable_validation_layers)
			destroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);

		vkDestroyInstance(instance, nullptr);
		//////////// END instance context

		glfwDestroyWindow(device->window);
		glfwTerminate();
		std::cout << "deleted Program Context" << std::endl;
	}

	void mainLoop() {
		while(!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

			render(
					  device, swap_chain
					, render_scene
					, framebuffer_resized
					, in_flight_fences[current_frame]
					, image_available_semaphores[current_frame], render_finished_semaphores[current_frame]
					, render_pipeline
					, current_frame
				);
			render_scene->viewport_extent = fromVkExtent(swap_chain->extent);
			// std::cout << "Frame Drawn!!" << std::endl;
		}

		vkDeviceWaitIdle(device->logical_device);
	}

	// FrameContext: current presentable image, sync prims, command buffers, framebuffers & attachments, memory of each, swapchain
	/*
	 * Deps: device, swapChain, command_buffers graphics_queue, present_queue
	 *		in_flight_fences, image_available_semaphores, render_finished_semaphores
	 *		current_frame, framebuffer_resized, MAX_FRAMES_IN_FLIGHT
	 *		refreshRenderContextPipelineSwapChain(), setUniformBuffer(), recordDrawCommandBuffer()
	 * Gets frame context, executes simulation step
	 *	, updates visor command buffers, presents frame context
	 *	 manages visor rendering context (graphics_pipeline, render pass)
	 */

	static void updateWorld(RenderScene * render_scene, BufferData<UniformBufferObject> & ubo) {
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		ubo.model = glm::rotate(glm::mat4(1.0), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		// ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(4.0f, 4.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f)
				, render_scene->viewport_extent.x / (float) render_scene->viewport_extent.y
				, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;

	}

	static void render(
			  const ComputeDevice * device
			, SwapChain * swap_chain

			, RenderScene * render_scene
			, bool & framebuffer_resized
			, VkFence render_fence, VkSemaphore image_available_semaphore, VkSemaphore render_finished_semaphore

			, RenderPipeline * render_pipeline
			, uint32_t current_frame
	) {

		vkWaitForFences(device->logical_device, 1, &render_fence, VK_TRUE, UINT64_MAX);

		uint32_t image_i;
		auto result = swap_chain->acquireNextImage(&image_i, image_available_semaphore);

		if(result == SwapChain::Result::ERROR_OUTDATED) {
			refreshRenderContextPipelineSwapChain(device, swap_chain, render_pipeline);
			return;
		}
		else if (result >= SwapChain::Result::ERROR)
			throw std::runtime_error("failed to acquire swap chain image!");
		
		BufferData<UniformBufferObject> world_ubo;
		updateWorld(render_scene, world_ubo);

		render_scene->uniform0_description = &world_ubo;
		render_scene->uniform0 = &world_ubo;

		render_pipeline->drawScene(render_fence, image_available_semaphore, render_finished_semaphore
				, image_i, current_frame
				);

		result = swap_chain->presentImage(&image_i, render_finished_semaphore);
		if(result == SwapChain::Result::ERROR_OUTDATED || result == SwapChain::Result::SUBOPTIMAL
				|| framebuffer_resized) {
			framebuffer_resized = false;
			refreshRenderContextPipelineSwapChain(device, swap_chain, render_pipeline);
		}
		else if (result >= SwapChain::ERROR) std::runtime_error("failed to present swap chain image!");
		
	}



	// LLRenderer:: || ComputeContext::
	// TODO: STRUCT::ComputeContext(sync, framebuffers)
	// "Renderer context" renderer <= camera?
	// Ups: image_available_semaphores, render_finished_semaphores, in_flight_fences
	void createSyncObjects(VkDevice device, const int frame_count) {

		image_available_semaphores.resize(frame_count);
		render_finished_semaphores.resize(frame_count);
		in_flight_fences.resize(frame_count);

		VkSemaphoreCreateInfo semaphore_info {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
		};
		VkFenceCreateInfo fence_info {
			  .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
			, .flags = VK_FENCE_CREATE_SIGNALED_BIT
		};
		
		for(size_t i = 0; i < frame_count; i++) {
			if(
				  vkCreateSemaphore(device, &semaphore_info, nullptr, &image_available_semaphores[i]) 
					!= VK_SUCCESS
				|| vkCreateSemaphore(device, &semaphore_info, nullptr, &render_finished_semaphores[i])
					!= VK_SUCCESS
				|| vkCreateFence(device, &fence_info, nullptr, &in_flight_fences[i]) 
					!= VK_SUCCESS
			  ) {
				throw std::runtime_error("failed to create semaphores for a frame!");
			}
		}

		std::cout << "Created semaphores" << std::endl;
	}

	// On window resize, etc
	static void refreshRenderContextPipelineSwapChain(
			const ComputeDevice * device
			, SwapChain * swap_chain
			// , VulkanLLRenderer & llrenderer
			, RenderPipeline * render_pipeline
	) {
		vkDeviceWaitIdle(device->logical_device);

		render_pipeline->destroy();

		delete swap_chain;

		swap_chain = new SwapChain(device);

		RenderPipeline::createRenderPipeline(device, swap_chain, render_pipeline);
		// createRenderPipeline(device, swap_chain, &llrenderer);

		std::cout << "recreated swap_chain" << std::endl;
	}

	/**
	 * APIInstance::
	 * @sideeffect instance
	 */
	static void createInstance(const std::vector<const char*> & validation_layers, VkInstance & instance) {

		if(!checkValidationLayerSupport(validation_layers))
			throw std::runtime_error("Validation layers requested, but not available!");
		if(!checkExtensionSupport(validation_layers)) throw std::runtime_error("Unsupported extensions");

		VkApplicationInfo application_info {
			  .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO
			, .pApplicationName = "GENERIC"
			, .applicationVersion = VK_MAKE_VERSION(1,0,0)
			, .pEngineName = "NEngine"
			, .engineVersion = VK_MAKE_VERSION(1,0,0)
			, .apiVersion = VK_API_VERSION_1_0
		};
		auto extensions = getRequiredExtensions(validation_layers);
		VkInstanceCreateInfo create_info {
			  .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
			, .flags = 0
			, .pApplicationInfo = &application_info
			, .enabledExtensionCount = static_cast<uint32_t>(extensions.size())
			, .ppEnabledExtensionNames = extensions.data()
		};
		// Validation Layers
		if(validation_layers.size()) {
			create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			create_info.ppEnabledLayerNames = validation_layers.data();
			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
			populateDebugMessengerCreateInfo(debugCreateInfo);
			create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		} else
			create_info.enabledLayerCount = 0; 

		// create_info.flags = 0;
		if(vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS)
			throw std::runtime_error("failed to create instance!");

		std::cout << "Vulkan instance created" << std::endl;

#ifdef DEBUG
		std::cout << "listing extensions: (count " << extensions.size() << ")" << std::endl;
		for (auto ex : extensions) std::cout << ex << std::endl;
		if(validation_layers.size()) std::cout << "Validation Layers added" << std::endl;
		else std::cout << "Validation Layers not added" << std::endl;
#endif
	}


	/*
	 * APIInstance::
	 * @sideeffect debug_messenger
	 */
	static void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT & debug_messenger) {

		VkDebugUtilsMessengerCreateInfoEXT create_info;
		populateDebugMessengerCreateInfo(create_info);
		if(createDebugUtilsMessengerEXT(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS)
			throw std::runtime_error("failed to set up debug messenger!");
	}

	// APIInstance
	// Deps: glfwGetRequiredInstanceExtensions()
	static std::vector<const char*> getRequiredExtensions(const std::vector<const char*> & validation_layers) {
		uint32_t glfw_extension_l {0};
		const char** glfw_extensions 
			= glfwGetRequiredInstanceExtensions(&glfw_extension_l);

		std::vector<const char*> extensions(
				glfw_extensions, glfw_extensions + glfw_extension_l);

		if(validation_layers.size())
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return extensions;
	}

	// APIInstance::
	// Deps: getRequiredExtensions(), vkEnumerateInstanceExtensionProperties()
	static bool checkExtensionSupport(const std::vector<const char*> validation_layers) {
		auto required_extensions = getRequiredExtensions(validation_layers);

		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
		std::vector<VkExtensionProperties> extensions(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

		std::cout << "Available extensions: \n";
		for(const auto& extension : extensions)
			std::cout << '\t' << extension.extensionName << '\n';

		std::vector<char*> unsatisfied_extensions;
		for(auto required_extension : required_extensions) {
			
			std::cout << '\t' << required_extension << '\n';

			bool found = false;
			for(const auto& extension : extensions)
				if(std::strcmp(required_extension, extension.extensionName)) 
					found = true;

			if(found == false) {
				unsatisfied_extensions.push_back((char*)required_extension);
				break;
			}
		}

		if(unsatisfied_extensions.size() > 0) {
			std::cout << std::endl << "unsatisfied extensions!:";
			for(auto extension : unsatisfied_extensions)
				std::cout << std::endl << extension;

			return false;
		}
		else 
			std::cout << "Vulkan Instance extensions meet." << std::endl;

		return true;
	}

};

}
