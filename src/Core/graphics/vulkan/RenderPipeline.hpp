#pragma once
#include "ComputeDevice.hpp"
#include "SwapChain.hpp"
#include "../RenderScene.hpp"
#include "../SceneResources.hpp"
#include "resources.hpp"

#include "../../util.hpp"

#include <vulkan/vulkan_core.h>

#include <array>
#include <vector>

namespace NEngine {

// SCENE GLOBALS
struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

// Holds buffers for each frame for potentially mutable data & parallel rendering
struct BufferFrame {
	const DataBuffer * buffer_data;

	const ComputeDevice * device;
	std::vector<VkBuffer> buffers;
	std::vector<VkDeviceMemory> memory;
	uint32_t frame_count = 0;

	BufferFrame(const ComputeDevice & device, const DataBuffer * data, uint32_t frame_count) 
		: device{&device}, buffer_data{data}, frame_count{frame_count}
	{
		createBuffers(device, buffer_data->size_bytes, frame_count);
	};
	~BufferFrame() { destroyBuffers(); };

	void createBuffers(const ComputeDevice & device, uint32_t size_bytes, uint32_t frame_count) {
		buffers.resize(frame_count);
		memory.resize(frame_count);

		for(size_t frame_i = 0; frame_i < frame_count; frame_i++) {
			ComputeDevice::createBuffer(device.logical_device, device.physical_device, size_bytes
					, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
					, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
					, buffers[frame_i], memory[frame_i]);
		}
	}
	void copyData() {
	}
	void destroyBuffers() {
		for(size_t frame_i = 0; frame_i < frame_count; frame_i++) {
			vkDestroyBuffer(device->logical_device, buffers[frame_i], nullptr);
			vkFreeMemory(device->logical_device, memory[frame_i], nullptr);
		}
	}
};

struct RenderPipeline {
	// providers
	const ComputeDevice * device;
	const SwapChain * swap_chain;
	VkDescriptorPool descriptor_pool;

	// own state
	VkRenderPass render_pass;
	std::vector<VkFramebuffer> framebuffers;

	VkDescriptorSetLayout descriptor_set_layout;
	std::vector<VkDescriptorSet> scene_descriptor_sets;
	std::vector<VkVertexInputBindingDescription> binding_descriptions;
	std::vector<VkVertexInputAttributeDescription> attribute_descriptions;

	VkShaderModule vert_shader_module;
	VkShaderModule frag_shader_module;

	VkPipeline graphics_pipeline;
	VkPipelineLayout graphics_pipeline_layout;

	bool use_swap_chain_viewport = true;

	// drawing stage state: mutable data owned by each frame
	uint32_t frame_count = 4;

	BufferFrame * uniform_buffer_frame = nullptr;
	// std::vector<VkBuffer> uniform_buffers;
	// std::vector<VkDeviceMemory> uniform_buffers_memory;
	std::vector<VkCommandBuffer> command_buffers;
	// we compute position cache (like the space graph leaf here for now)
	// "per object variable data" -> use single buffer
	std::vector<std::vector<EntitySpatialComponent>> leaf_position_cache;
	size_t entity_buffer_size = 0;
	size_t entity_buffer_block_size = 0;
	size_t entity_buffer_effective_size = 0;
	// std::vector<std::vector<VkBuffer>> leaf_position_cache_buffer;
	// std::vector<std::vector<VkDeviceMemory>> leaf_position_cache_memory;
	std::vector<VkBuffer> leaf_position_cache_buffer;
	std::vector<VkDeviceMemory> leaf_position_cache_memory;
	
	RenderScene * render_scene;


	// all in bytes number
	size_t padMinByteSize(size_t min_alignment, size_t original_size) {
		return min_alignment > 0 
			? (original_size + min_alignment - 1) & ~(min_alignment - 1)
			: original_size
			;
	}

	// TODO: separate layout creation, textures, then optimize positions

	// separate scene description layout from swap changes
	RenderPipeline(
		const ComputeDevice * device, const SwapChain * swap_chain
		, uint32_t frame_count // render pipeline specific args
		, RenderScene * render_scene // current scene
	) : device{device}//, descriptor_pool{descriptor_pool}
	, frame_count{frame_count}, render_scene{render_scene} 
	{
		std::cout << "ctor render pipeline" << std::endl;

		entity_buffer_effective_size = sizeof(EntitySpatialComponent);
		entity_buffer_block_size = padMinByteSize(
				entity_buffer_effective_size, device->properties.limits.minUniformBufferOffsetAlignment);
		entity_buffer_size = entity_buffer_block_size*render_scene->entities.size();

		std::cout << "parsing scene" << std::endl;
		this->setScene(render_scene);
		std::cout << "scene set" << std::endl;
		
		vert_shader_module = device->createShaderModule(device->logical_device, render_scene->vert_shader_code);
		frag_shader_module = device->createShaderModule(device->logical_device, render_scene->frag_shader_code);

		render_scene->vertices_descriptions = {render_scene->entities[0].render_type->mesh->vertices.description};

		RenderPipeline::getVertexInputDescriptions(
				render_scene->vertices_descriptions, binding_descriptions, attribute_descriptions);

		// scene -> uniform buffer, entity type list -> texture
		const size_t entity_count = render_scene->entities.size();
		createDescriptorSetLayout(device->logical_device, descriptor_set_layout, entity_count);

		createDescriptorPool(device->logical_device, this->descriptor_pool, frame_count);

		createDescriptorSets(device->logical_device, this->descriptor_pool
			, descriptor_set_layout
			, scene_descriptor_sets
			, uniform_buffer_frame->buffers
			, uniform_buffer_frame->buffer_data->size_bytes
			, leaf_position_cache_buffer, entity_buffer_block_size
			, render_scene->entities[0].render_type->material->texture->device_data->sampler
			, render_scene->entities[0].render_type->material->texture->device_data->view
			, frame_count
			, entity_count);
		
		std::cout << "creating render pipeline" << std::endl;
		createRenderPipeline(device, swap_chain, this);

		device->allocateCommandBuffers(command_buffers, frame_count);
	}

	// release:
	/*
		vkDestroyDescriptorPool(device->logical_device, descriptor_pool, nullptr);
	*/

	void setScene(RenderScene * render_scene) {
		render_scene->vert_shader_code = fs::readFile(render_scene->vert_shader_path);
		render_scene->frag_shader_code = fs::readFile(render_scene->frag_shader_path);

		// load entities into device
		std::cout << "loading entities" << render_scene->entities.size() << std::endl;
		for(auto entity : render_scene->entities) {
			std::cout << "loading entity" << render_scene->entities.size() << std::endl;
			entity.render_type->load(device);
		}
		std::cout << "entities loaded" << std::endl;

		// create dense caches: positional
		resizePositionCache();
		for(int i = 0; i < frame_count; i++)
			updatePositionCache(i);

		if(uniform_buffer_frame != nullptr) delete uniform_buffer_frame;

		uniform_buffer_frame = new BufferFrame(*device, render_scene->scene_buffer, frame_count);
	}

	void resizePositionCache() {
		leaf_position_cache_buffer.resize(frame_count);
		leaf_position_cache_memory.resize(frame_count);

		for(size_t frame_i = 0; frame_i < frame_count; frame_i++) {

			ComputeDevice::createBuffer(device->logical_device, device->physical_device
					, entity_buffer_size
					, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
					, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
					, leaf_position_cache_buffer[frame_i]
					, leaf_position_cache_memory[frame_i]);
		}
			std::cout << "created entity buffer with size " << entity_buffer_size;
			std::cout << "created entity buffer with block size " << entity_buffer_block_size;
	}

	// TODO change into physics/transform system space graph render flag layer
	// TODO convert to single buffer write
	void updatePositionCache(size_t current_index) {
		for(size_t entity_i = 0; entity_i < render_scene->entities.size(); entity_i++) {
			// leaf_position_cache[i] = *render_scene->entities[i].spatial_component;
			ComputeDevice::setDeviceMemory(device->logical_device
				, render_scene->entities[entity_i].spatial_component
				, entity_buffer_effective_size
				, leaf_position_cache_memory[current_index]
				, entity_buffer_block_size*entity_i
				);
		}
	}
	void copyPositionCache() {

	}

	void refreshUniformBufferFrame() {
		uniform_buffer_frame->copyData();
	}

	RenderPipeline(
		const ComputeDevice * device, const SwapChain * swap_chain, const RenderScene * render_scene
	) {

			// createRenderPipeline(device->physical_device, device->logical_device, device->command_pool, device-> graphics_queue, swap_chain->extent, swap_chain->swap_chain_image_format, swap_chain->depth_format, swap_chain->swap_chain_image_views, swap_chain->depth_image_view, render_pass, framebuffer, )
	}

	// TODO: split into set of changes and refresh(changes) for constructor = refresh(any combination) && simplify 
	// transitions, keep this ugly remap until then
	// @sideeffect this::{}
	static void createRenderPipeline(
		  const ComputeDevice * device, const SwapChain * swap_chain, RenderPipeline * render_pipeline
	) {

		
		createRenderPass(device->logical_device, swap_chain->swap_chain_image_format, swap_chain->depth_format, render_pipeline->render_pass);
		createRenderPipelineFramebuffer(device->logical_device, device->physical_device
				, device->command_pool, device->graphics_queue
				, render_pipeline->render_pass, swap_chain->swap_chain_image_views, swap_chain->extent
				, swap_chain->depth_image_view
				, render_pipeline->framebuffers);
		createGraphicsPipeline(device->logical_device, render_pipeline->render_pass
				, swap_chain->extent
				, render_pipeline->descriptor_set_layout
				, render_pipeline->binding_descriptions, render_pipeline->attribute_descriptions
				, render_pipeline->vert_shader_module, render_pipeline->frag_shader_module, render_pipeline->graphics_pipeline_layout, render_pipeline->graphics_pipeline);
	}

	// @sideeffect this::{}
	// static void createRenderPipeline(
		  // const ComputeDevice * device, const SwapChain * swap_chain, VulkanLLRenderer * llrenderer
//
	// ) {
		// createRenderPass(device->logical_device, swap_chain->swap_chain_image_format, swap_chain->depth_format, llrenderer->render_pass);
		// createRenderPipelineFramebuffer(device->logical_device, device->physical_device
				// , device->command_pool, device->graphics_queue
				// , llrenderer->render_pass, swap_chain->swap_chain_image_views, swap_chain->extent
				// , swap_chain->depth_image_view
				// , llrenderer->render_pipeline_framebuffers);
		// createGraphicsPipeline(device->logical_device, llrenderer->render_pass
				// , swap_chain->extent, llrenderer->descriptor_set_layout, llrenderer->binding_descriptions, llrenderer->attribute_descriptions
				// , llrenderer->vert_shader_module, llrenderer->frag_shader_module, llrenderer->graphics_pipeline_layout, llrenderer->graphics_pipeline);
//
		// // createRenderPipeline(device->physical_device, device->logical_device, device->command_pool, device->graphics_queue,
				// // , swap_chain->extent, swap_chain->swap_chain_image_format, swap_chain->depth_format, swap_chain->swap_chain_image_views, swap_chain->depth_image_view, llr
	// }

	// Destroys current constructed pipeline: render_pass, framebuffer, graphics_pipeline & layout
	// @sideeffect render_pass, framebuffer, graphics_pipeline, graphics_pipeline_layout
	void destroy() {
		cleanRenderPipeline(device->logical_device
			, render_pass, framebuffers, graphics_pipeline, graphics_pipeline_layout);
	}

	void drawScene(
			VkFence render_fence, VkSemaphore image_available_semaphore, VkSemaphore render_finished_semaphore
			, uint32_t image_i, uint32_t current_frame
			) {
		this->drawScene(
			  this->device, render_fence, image_available_semaphore, render_finished_semaphore
			, this->render_pass, this->framebuffers[image_i], this->graphics_pipeline, this->graphics_pipeline_layout
			, this->command_buffers[current_frame]
			, this->render_scene
			, this->uniform_buffer_frame->memory[current_frame]
			, this->scene_descriptor_sets[current_frame]
			, this->entity_buffer_block_size
		);
	}

	static void drawScene(
			  const ComputeDevice * device
			, VkFence render_fence, VkSemaphore image_available_semaphore, VkSemaphore render_finished_semaphore
			// RenderPipeline
			, VkRenderPass render_pass, VkFramebuffer render_pipeline_framebuffer
			, VkPipeline graphics_pipeline, VkPipelineLayout graphics_pipeline_layout
			, VkCommandBuffer command_buffer
			// world step
			, RenderScene * render_scene
			// object-level draw arguments RenderContext ?
			, VkDeviceMemory uniform_buffer_memory, VkDescriptorSet descriptor_set
			, uint32_t binding_offset
	) {

		// Update world state & uniform buffer
		// TODO: move to simulation world data

		// render_scene->scene_buffer = new

		ComputeDevice::setDeviceMemory(device->logical_device
				, render_scene->uniform0, render_scene->uniform0_description->size_bytes, uniform_buffer_memory);

		// render_pipeline->updateUniformBu

		// Reset/Submit Command Buffer
		vkResetCommandBuffer(command_buffer, 0);
		recordDrawCommandBuffer(command_buffer
				, render_pass, render_pipeline_framebuffer, toVkExtent(render_scene->viewport_extent)
				, graphics_pipeline, graphics_pipeline_layout, descriptor_set
				, render_scene->entities[0].render_type->mesh->vertices.device_data->buffer
				, render_scene->entities[0].render_type->mesh->indices.device_data->buffer 
				, render_scene->entities[0].render_type->mesh->indices.size
				, render_scene->entities
				, binding_offset
				);
		VkSemaphore wait_sempahores[] {image_available_semaphore};
		VkPipelineStageFlags wait_stages[] {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		VkSemaphore signal_semaphores[] {render_finished_semaphore};
		VkSubmitInfo submit_info {
			  .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO
			, .waitSemaphoreCount = 1
			, .pWaitSemaphores = wait_sempahores
			, .pWaitDstStageMask = wait_stages
			, .commandBufferCount = 1
			, .pCommandBuffers = &command_buffer
			, .signalSemaphoreCount = 1
			, .pSignalSemaphores = signal_semaphores
		};

		vkResetFences(device->logical_device, 1, &render_fence); // TODO: create new render_fence in-place
		if(vkQueueSubmit(device->graphics_queue, 1, &submit_info, render_fence) != VK_SUCCESS)
			throw std::runtime_error("failed to submit draw command buffer");
	}

	// LLRenderer::
	// @sideeffect command_buffer: Begins command_buffer, render_pass
	//		, binds graphics_pipeline, vertex, index buffers, descriptor_set
	//		, then draws indexed, ends render_pass command_buffer
	static void recordDrawCommandBuffer(
			  VkCommandBuffer command_buffer
			, VkRenderPass render_pass, VkFramebuffer framebuffer, VkExtent2D render_area_offset
			, VkPipeline graphics_pipeline, VkPipelineLayout pipeline_layout
			, VkDescriptorSet descriptor_set
			, VkBuffer vertex_buffer, VkBuffer index_buffer, uint32_t index_count
			, std::vector<EntityRenderComponent> entities
			, uint32_t binding_offset
	) {

		VkCommandBufferBeginInfo begin_info {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
		// begin_info.flags = 0; // optional
		// begin_info.pInheritanceInfo = nullptr; // optional
		if(vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
			throw std::runtime_error("failed to begin recording command buffer!");

		std::array<VkClearValue, 2> clear_values {{
			  {.color = {{0.2f, 0.2f, 0.2f, 1.0f}}}
			, {.depthStencil = {1.0f, 0}}
		}};
		VkRenderPassBeginInfo render_pass_begin_info {
			  .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO
			, .renderPass = render_pass
			, .framebuffer = framebuffer
			, .renderArea = {
				  .offset = {0, 0}
				, .extent = render_area_offset
			}
			, .clearValueCount = static_cast<uint32_t>(clear_values.size())
			, .pClearValues = clear_values.data()
		};
		vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

		////////////////////////////////// ENTITY LOOP
		for(uint32_t entity_i = 0; entity_i < entities.size(); entity_i++) {
			const EntityRenderType render_type = *entities[entity_i].render_type;

			VkBuffer vertex_buffers[] = {render_type.mesh->vertices.device_data->buffer};
			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
			vkCmdBindIndexBuffer(command_buffer, render_type.mesh->indices.device_data->buffer, 0, VK_INDEX_TYPE_UINT16);

			// TODO: build renderer arround command buffer optimization
			// https://www.reddit.com/r/vulkan/comments/g14e82/proper_way_to_use_commandbuffers/
			uint32_t entity_offset = binding_offset*entity_i;
			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS
					, pipeline_layout, 0, 1, &descriptor_set, 1, &entity_offset);
			vkCmdDrawIndexed(command_buffer, render_type.mesh->indices.size, 1, 0, 0, 0);

		}

		vkCmdEndRenderPass(command_buffer);

		if(vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
			throw std::runtime_error("failed to record command buffer!");
	}

	// @sideeffect uniform_buffers, uniform_buffers_memory resize(frame_count), then created with createBuffer
	static void createFramesUniformBuffers(
			  VkDevice device, VkPhysicalDevice physical_device
			, std::vector<VkBuffer> & buffers, std::vector<VkDeviceMemory> & buffers_memory
			, VkDeviceSize buffer_size, uint32_t frame_count
	) {

		buffers.resize(frame_count);
		buffers_memory.resize(frame_count);

		for(size_t frame_i = 0; frame_i < frame_count; frame_i++) {
			ComputeDevice::createBuffer(device, physical_device, buffer_size
					, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
					, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
					, buffers[frame_i], buffers_memory[frame_i]);
		}
	}

	/**
	 * LLRenderer::
	 * creates descriptor set layout for current scene descriptors
	 * @sideeffect descriptor_set_layout
	 */
	static void createDescriptorSetLayout(VkDevice device
			, VkDescriptorSetLayout & descriptor_set_layout
			, size_t entity_count
	) {
		VkDescriptorSetLayoutBinding ubo_layout_binding {
			  .binding = 0
			, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
			, .descriptorCount = 1
			, .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
			, .pImmutableSamplers = nullptr
		};
		VkDescriptorSetLayoutBinding texture_sampler_layout_binding {
			  .binding = 1
			, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
			, .descriptorCount = 1
			, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
			, .pImmutableSamplers = nullptr
		};
		VkDescriptorSetLayoutBinding position_layout_binding {
			  .binding = 2
			, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
			, .descriptorCount = 1
			, .stageFlags = VK_SHADER_STAGE_VERTEX_BIT  | VK_SHADER_STAGE_FRAGMENT_BIT
			, .pImmutableSamplers = nullptr
		};
		std::vector<VkDescriptorSetLayoutBinding> bindings = {
			ubo_layout_binding, texture_sampler_layout_binding, position_layout_binding
		};
		// std::array<VkDescriptorSetLayoutBinding, 3> bindings = {
			// ubo_layout_binding, texture_sampler_layout_binding, position_layout_binding
		// };

		VkDescriptorSetLayoutCreateInfo layout_info{
			  .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
			, .bindingCount = static_cast<uint32_t>(bindings.size())
			, .pBindings = bindings.data()
		};

		if(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &descriptor_set_layout) != VK_SUCCESS)
			throw std::runtime_error("failed to create descriptor set layout!");

		// layout_info = {
			  // .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
			// , .bindingCount = 1
			// , .pBindings = &position_layout_binding
		// };
//
		// if(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &per_object_layout) != VK_SUCCESS)
			// throw std::runtime_error("failed to create descriptor set layout!");
	}

	// TODO convert to pool "Map" with counters and so
	static void createDescriptorPool(VkDevice device, VkDescriptorPool &descriptor_pool, const uint32_t frame_count) {

		std::vector<VkDescriptorPoolSize> pool_sizes {{
			{
				  .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
				, .descriptorCount = frame_count*30
			}
			, { 
				  .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
				, .descriptorCount = frame_count*30
			}
			, { 
				  .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
				, .descriptorCount = frame_count*30
			}
		}};
		VkDescriptorPoolCreateInfo pool_info {
			  .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO
			// , .flags = 0;
			, .maxSets = frame_count
			, .poolSizeCount = static_cast<uint32_t>(pool_sizes.size())
			, .pPoolSizes = pool_sizes.data()
		};
		if(vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool) != VK_SUCCESS)
			throw std::runtime_error("failed to create descriptor pool");
	}

	// LLRenderer::
	// Allocates descriptor sets, writes uniforms, texture descriptors, for each frame.
	// @sideeffect descriptor_sets is resized, allocated, and the new descriptors are written into
	static void createDescriptorSets(
			  VkDevice device, VkDescriptorPool &descriptor_pool
			, const VkDescriptorSetLayout &descriptor_set_layout
			, std::vector<VkDescriptorSet> &descriptor_sets
			, const std::vector<VkBuffer> &uniform_buffers, VkDeviceSize uniform_buffer_size
			, const std::vector<VkBuffer> &position_cache, VkDeviceSize position_cache_size_bytes
			, VkSampler texture_sampler, VkImageView texture_image_view
			, const uint32_t frame_count
			, const uint32_t entity_count
	) {

		descriptor_sets.resize(frame_count);

		std::vector<VkDescriptorSetLayout> layouts(frame_count, descriptor_set_layout);
		VkDescriptorSetAllocateInfo alloc_info {
			  .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO
			, .descriptorPool = descriptor_pool
			, .descriptorSetCount = frame_count
			, .pSetLayouts = layouts.data()
		};
		if(vkAllocateDescriptorSets(device, &alloc_info, descriptor_sets.data()) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate descriptor sets");

		for(size_t frame_i = 0; frame_i < frame_count; frame_i++) {

			VkDescriptorBufferInfo buffer_info {
				  .buffer = uniform_buffers[frame_i]
				, .offset = 0
				, .range = uniform_buffer_size
			};
			VkDescriptorImageInfo image_info {
				  .sampler = texture_sampler
				, .imageView = texture_image_view
				, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
			VkDescriptorBufferInfo entity_buffer {
				  .buffer = position_cache[frame_i]
				, .offset = 0
				, .range = position_cache_size_bytes
			};
			std::vector<VkWriteDescriptorSet> descriptor_writes{{
				{
					  .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
					, .dstSet = descriptor_sets[frame_i]
					, .dstBinding = 0
					, .dstArrayElement = 0
					, .descriptorCount = 1
					, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
					, .pImageInfo = nullptr
					, .pBufferInfo = &buffer_info
					, .pTexelBufferView = nullptr
				}
				, {
					  .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
					, .dstSet = descriptor_sets[frame_i]
					, .dstBinding = 1
					, .dstArrayElement = 0
					, .descriptorCount = 1
					, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
					, .pImageInfo = &image_info
					, .pBufferInfo = nullptr
					, .pTexelBufferView = nullptr
				}
				, {
					  .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
					, .dstSet = descriptor_sets[frame_i]
					, .dstBinding = 2
					, .dstArrayElement = 0
					, .descriptorCount = 1
					, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
					, .pImageInfo = nullptr
					, .pBufferInfo = &entity_buffer
					, .pTexelBufferView = nullptr
				}
			}};

			// per-object type
			// for(size_t position_i = 0; position_i < position_cache[frame_i].size(); position_i++) {
				// descriptor_writes.push_back({
					  // .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
					// , .dstSet = descriptor_sets[frame_i]
					// , .dstBinding = 2
					// , .dstArrayElement = 0
					// , .descriptorCount = 1
					// , .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
					// , .pImageInfo = nullptr
					// , .pBufferInfo = &entity_buffer
					// , .pTexelBufferView = nullptr
				// });
			// }
			
			vkUpdateDescriptorSets(device, descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);


		}

		std::cout << "Created descriptor sets" << std::endl;
	}


	/**
	 * LLRenderer::
	 * Defines render_pass, single fragment shader stage subpass
	 * @sideeffect render_pass
	 */
	static void createRenderPass(VkDevice device
			, VkFormat color_format, VkFormat depth_format
			, VkRenderPass & render_pass
	) {
		VkAttachmentDescription color_attachment {
			  .format = color_format
			, .samples = VK_SAMPLE_COUNT_1_BIT
			, .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR
			, .storeOp = VK_ATTACHMENT_STORE_OP_STORE
			, .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE
			, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE
			, .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
			, .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};
		VkAttachmentDescription depth_attachment {
			  .format = depth_format
			, .samples = VK_SAMPLE_COUNT_1_BIT
			, .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR
			, .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE
			, .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR
			, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE
			, .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
			, .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};
		std::array<VkAttachmentDescription, 2> attachments = {color_attachment, depth_attachment};

		VkAttachmentReference color_attachment_ref {
			  .attachment = 0
			, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};
		VkAttachmentReference depth_attachment_ref {
			  .attachment = 1
			, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};
		VkSubpassDescription subpass {
			  .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS
			, .colorAttachmentCount = 1
			, .pColorAttachments = &color_attachment_ref
			, .pDepthStencilAttachment = &depth_attachment_ref
		};

		VkSubpassDependency dependency {
			  .srcSubpass = VK_SUBPASS_EXTERNAL
			, .dstSubpass = 0
			, .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 
							| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
			, .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 
							| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
			, .srcAccessMask = 0
			, .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
		};
		
		VkRenderPassCreateInfo render_pass_info {
			  .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO
			, .attachmentCount = static_cast<uint32_t>(attachments.size())
			, .pAttachments = attachments.data()
			, .subpassCount = 1
			, .pSubpasses = &subpass
			, .dependencyCount = 1
			, .pDependencies = &dependency
		};

		if(vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		std::cout << "Created render pass!" << std::endl;
	}

	/**
	 * LLRenderer::
	 * @sideeffect render_pipeline_framebuffers
	 */
	static void createRenderPipelineFramebuffer(
			  VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue graphics_queue
			, VkRenderPass render_pass
			, const std::vector<VkImageView> & swap_chain_image_views, const VkExtent2D & extent
			, const VkImageView depth_image_view
			, std::vector<VkFramebuffer> & render_pipeline_framebuffers
	) {

		render_pipeline_framebuffers.resize(swap_chain_image_views.size());

		for(size_t i = 0; i < swap_chain_image_views.size(); i++) {
			std::array<VkImageView, 2> attachments = {
				  swap_chain_image_views[i]
				, depth_image_view
			};

			VkFramebufferCreateInfo framebuffer_info {
				  .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO
				, .renderPass = render_pass
				, .attachmentCount = static_cast<uint32_t>(attachments.size())
				, .pAttachments = attachments.data()
				, .width = extent.width
				, .height = extent.height
				, .layers = 1
			};

			if(vkCreateFramebuffer(device, &framebuffer_info, nullptr, &render_pipeline_framebuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create framebuffer!");

			std::cout << "Created framebuffer!" << std::endl;
		}
	}

	/*
	 * LLRenderer::
	 * creates graphics graphics_pipeline using scenegraph info
	 * @sideeffect graphics_pipeline, pipeline_layout
	 */
	static void createGraphicsPipeline(
			  VkDevice device, VkRenderPass render_pass
			, VkExtent2D extent, const VkDescriptorSetLayout & descriptor_set_layout
			, const std::vector<VkVertexInputBindingDescription> & binding_descriptions
			, const std::vector<VkVertexInputAttributeDescription> & attribute_descriptions
			, const VkShaderModule & vert_shader_module, const VkShaderModule & frag_shader_module 
			, VkPipelineLayout & graphics_pipeline_layout, VkPipeline & graphics_pipeline
	) {

		VkPipelineShaderStageCreateInfo stages_info[] = {
			{
				  .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
				, .stage = VK_SHADER_STAGE_VERTEX_BIT
				, .module = vert_shader_module
				, .pName = "main"
			}, {
				  .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
				, .stage = VK_SHADER_STAGE_FRAGMENT_BIT
				, .module = frag_shader_module
				, .pName = "main"
			}
		};

		// Vertex input
		VkPipelineVertexInputStateCreateInfo vertex_input_info {
			  .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
			, .vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size())
			, .pVertexBindingDescriptions = binding_descriptions.data() // optional
			, .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size())
			, .pVertexAttributeDescriptions = attribute_descriptions.data() // optional
		};
		// input Assembly
		VkPipelineInputAssemblyStateCreateInfo input_assembly_info {
			  .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
			, .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
			, .primitiveRestartEnable = VK_FALSE
		};

		// (viewport, scissors, viewportState), rasterizer, multisampling
		VkViewport viewport {
			  .x = 0.0f
			, .y = 0.0f
			, .width = (float) extent.width
			, .height = (float) extent.height
			, .minDepth = 0.0f
			, .maxDepth = 1.0f
		};
		VkRect2D scissor {
			  .offset = {0, 0}
			, .extent = extent
		};
		VkPipelineViewportStateCreateInfo viewport_info {
			  .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
			, .viewportCount = 1
			, .pViewports = &viewport
			, .scissorCount = 1
			, .pScissors = &scissor
		};
		VkPipelineRasterizationStateCreateInfo rasterization_info {
			  .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
			, .depthClampEnable = VK_FALSE
			, .rasterizerDiscardEnable = VK_FALSE
			, .polygonMode = VK_POLYGON_MODE_FILL
			, .cullMode = VK_CULL_MODE_BACK_BIT
			, .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE
			, .depthBiasEnable = VK_FALSE
			, .depthBiasConstantFactor = 0.0f	// optional
			, .depthBiasClamp = 0.0f			// optional
			, .depthBiasSlopeFactor = 0.0f		// optional
			, .lineWidth = 1.0f
		};
		VkPipelineMultisampleStateCreateInfo multisample_info {
			  .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
			, .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
			, .sampleShadingEnable = VK_FALSE
			, .minSampleShading = 1.0f	// optional
			, .pSampleMask = nullptr	// optional
			, .alphaToCoverageEnable = VK_FALSE // optional
			, .alphaToOneEnable = VK_FALSE		// optional
		};

		// Depth/Stencil testing, color blending
		VkPipelineDepthStencilStateCreateInfo depth_stencil_info {
			  .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
			, .depthTestEnable = VK_TRUE
			, .depthWriteEnable = VK_TRUE
			, .depthCompareOp = VK_COMPARE_OP_LESS
			, .depthBoundsTestEnable = VK_FALSE
			, .stencilTestEnable = VK_FALSE
			, .front = {}
			, .back = {}
			, .minDepthBounds = 0.0f
			, .maxDepthBounds = 1.0f
		};
		VkPipelineColorBlendAttachmentState color_blend_attachment{
			  .blendEnable = VK_FALSE
			, .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
								| VK_COLOR_COMPONENT_A_BIT
			// , .srcColorBlendFactor = VK_BLEND_FACTOR_ONE // optional
			// , .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO // optional
			// , .colorBlendOp = VK_BLEND_OP_ADD // optional
			// , .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE // optional
			// , .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO // optional
			// , .alphaBlendOp = VK_BLEND_OP_ADD // optional
			// ------ ALPHA HANDLING
			//	 .blendEnable = VK_TRUE
			// , .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA // optional
			// , .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA // optional
			// , .colorBlendOp = VK_BLEND_OP_ADD // optional
			// , .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE // optional
			// , .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO // optional
			// , .alphaBlendOp = VK_BLEND_OP_ADD // optional
		};
		VkPipelineColorBlendStateCreateInfo color_blending_info {
			  .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO
			, .logicOpEnable = VK_FALSE
			, .logicOp = VK_LOGIC_OP_COPY // optional
			, .attachmentCount = 1
			, .pAttachments = &color_blend_attachment
			, .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f} // optional
		};

		// Dynamic State
		/*
		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};
		VkPipelineDynamicStateCreateInfo dynamicState {
			  .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
			, .dynamicStateCount = 2
			, .pDynamicStates = dynamicStates
		};
		*/

		std::vector<VkDescriptorSetLayout> descriptor_set_layouts {
			descriptor_set_layout
		};

		// Pipeline Layout
		VkPipelineLayoutCreateInfo pipeline_layout_info {
			  .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
			, .setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size())
			, .pSetLayouts = descriptor_set_layouts.data()
			, .pushConstantRangeCount = 0
			// , .pPushConstantRanges = nullptr
		};
		if(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &graphics_pipeline_layout) != VK_SUCCESS)
			throw std::runtime_error("failed to create graphics_pipeline layout!");

		// Pipeline
		VkGraphicsPipelineCreateInfo pipeline_info {
			  .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO
			, .stageCount = 2
			, .pStages = stages_info

			, .pVertexInputState = &vertex_input_info
			, .pInputAssemblyState = &input_assembly_info

			, .pViewportState = &viewport_info
			, .pRasterizationState = &rasterization_info
			, .pMultisampleState = &multisample_info

			, .pDepthStencilState = &depth_stencil_info
			, .pColorBlendState = &color_blending_info

			, .pDynamicState = nullptr
			, .layout = graphics_pipeline_layout
			, .renderPass = render_pass
			, .subpass = 0
			, .basePipelineHandle = VK_NULL_HANDLE
			, .basePipelineIndex = -1
		};
		if(vkCreateGraphicsPipelines(
					device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline) != VK_SUCCESS)
			throw std::runtime_error("failed to create graphics graphics_pipeline!");

		vkDestroyShaderModule(device, frag_shader_module, nullptr);
		vkDestroyShaderModule(device, vert_shader_module, nullptr);

		std::cout << "Created graphics graphics_pipeline!" << std::endl;
	}

	// @sideeffect render_pass, render_pipeline_framebuffers, graphics_pipeline, graphics_pipeline_layout
	static void cleanRenderPipeline(
			  VkDevice logical_device
			, VkRenderPass & render_pass 
			, std::vector<VkFramebuffer> & render_pipeline_framebuffers
			, VkPipeline & graphics_pipeline, VkPipelineLayout & graphics_pipeline_layout
	) {
		// Pipeline
		vkDestroyPipeline(logical_device, graphics_pipeline, nullptr);
		vkDestroyPipelineLayout(logical_device, graphics_pipeline_layout, nullptr);

		for(size_t i = 0; i < render_pipeline_framebuffers.size(); i++) {
			vkDestroyFramebuffer(logical_device, render_pipeline_framebuffers[i], nullptr);
		}
		vkDestroyRenderPass(logical_device, render_pass, nullptr); // only if swapchainImageFormat changes
	}

	static void getVertexInputDescriptions(
			  const std::vector<VertexDataDescription*> & vertex_description_list
			, std::vector<VkVertexInputBindingDescription> & binding_descriptions
			, std::vector<VkVertexInputAttributeDescription> & attribute_descriptions
	) {

		uint32_t list_length = vertex_description_list.size();
		uint32_t attribute_offset = 0;
		binding_descriptions.resize(list_length);

		for(uint32_t descriptor_i=0; descriptor_i<list_length; descriptor_i++) {
			VertexDataDescription * vertex_description = vertex_description_list[descriptor_i];

			uint32_t binding = descriptor_i;

			binding_descriptions[descriptor_i] = {
				  .binding = binding
				, .stride = vertex_description->stride
				, .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
			};
			
			uint32_t attribute_n = vertex_description->attributes.size();
			attribute_descriptions.resize(attribute_offset + attribute_n);

			for(uint32_t att_i = 0; att_i < attribute_n; att_i++) {
				uint32_t location = attribute_offset;

				attribute_descriptions[attribute_offset] = {
					  .location = location
					, .binding = binding
					, .format = toVkFormat(vertex_description->attributes[att_i].format)
					, .offset = vertex_description->attributes[att_i].offset
				};

				attribute_offset += 1;
			}
		}
// #ifdef DEBUG
		// printVertexInputDescriptions(binding_descriptions, attribute_descriptions);
// #endif
	}

	static void printVertexInputDescriptions(
			  std::vector<VkVertexInputBindingDescription> * binding_descriptions
			, std::vector<VkVertexInputAttributeDescription> * attribute_descriptions
	) {

		uint32_t bindings_l = binding_descriptions->size();
		uint32_t attributes_l = attribute_descriptions->size();

		for(uint32_t binding_i = 0; binding_i < bindings_l; binding_i++) {
			VkVertexInputBindingDescription binding = (*binding_descriptions)[binding_i];
			std::cout << "BindingDescription: " << binding_i << std::endl;
			std::cout << "binding: " << binding.binding << std::endl;
			std::cout << "stride: " << binding.stride << std::endl;
			std::cout << "inputRate: " << binding.inputRate << std::endl;
		}

		for(uint32_t attribute_i = 0; attribute_i < attributes_l; attribute_i++) {
			VkVertexInputAttributeDescription attribute = (*attribute_descriptions)[attribute_i];
				std::cout << "AttributeDescription: " << attribute_i << std::endl;
				std::cout << "location: " << attribute.location << std::endl;
				std::cout << "binding: " << attribute.binding << std::endl;
				std::cout << "format: " << attribute.format << std::endl;
				std::cout << "offset: " << attribute.offset << std::endl;
		}
	}

	};
}

