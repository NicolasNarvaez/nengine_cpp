#pragma once

#include "HostImage.hpp"

#ifdef NENGINE_LLRENDERER_VENDOR_VULKAN
#include "vulkan/DeviceImage.hpp"
#include "vulkan/DeviceVertexBuffer.hpp"
#include "vulkan/DeviceShaderModule.hpp"
#include "vulkan/ComputeDevice.hpp"
#endif

#include "resources.hpp"

#include <string>

#include "../os/fs.hpp"
// #define STB_IMAGE_IMPLEMENTATION
// #include <stb/stb_image.h>

namespace NEngine {
// All resources from the outside can be mapped to a taxonomical path, be it implemented on a fs, web or 
// dynamic, to decouple load. Handles should have a strong link to the path and be unique. TODO map

struct ShaderModule {
	std::string path;
	std::string code;
	DeviceShaderModule * device_data;

	ShaderModule(std::string path) : path{path} {};

	void loadCode() {};

	// creates shader module
	void load(const ComputeDevice * device) {
		// if(device_data == nullptr) device_data = new DeviceShaderModule(code, device);
	};
	void unload() {};
};

// NEngine nd-image wrapper
// Holds host, device memory pointers, uniquely represents resource. Includes generator interface.
struct Image {
	HostImage * image = nullptr;
	DeviceImage * device_data = nullptr;

	// TODO: add format auto-detector
	Image(Format format, const std::string path) : image{new HostImage(format, path)} {};

	// makes local copy
	Image(Format format, const void * data, size_t size_bytes, uint32_t width, uint32_t height) 
		: image{new HostImage(format, data, size_bytes, width, height)} {};

	void load(const ComputeDevice * device) {
		if(device_data != nullptr) return;

		std::cout << "loading image" << image->size_bytes << ", " << image->height << ", " << image->width << std::endl;
		image->load();
		std::cout << "creating device image" << std::endl;
		device_data = new DeviceImage(device, image);
	};
	void unload() {};
};

struct Material {
	Image * texture;
	ShaderModule * fragment_shader;
	ShaderModule * vertex_shader;
	Material(Format format, std::string texture_path, std::string vert_shader_path, std::string frag_shader_path) 
		: texture{new Image(format, texture_path)}
		, vertex_shader{new ShaderModule(vert_shader_path)}
		, fragment_shader{new ShaderModule(frag_shader_path)} {};

	Material(Image * src_image, std::string vert_shader_path, std::string frag_shader_path) 
		: texture{src_image}
		, vertex_shader{new ShaderModule(vert_shader_path)}
		, fragment_shader{new ShaderModule(frag_shader_path)} {};
	
	void load(const ComputeDevice * device) {
		std::cout << "loading material" << std::endl;
		texture->load(device); 
		std::cout << "loading fragment shader" << std::endl;
		fragment_shader->load(device); 
		std::cout << "loading vertex shader" << std::endl;
		vertex_shader->load(device); 
		std::cout << "material loaded" << std::endl;
	};
};

// Buffer object
struct DataBuffer {
	void * data;
	BufferDataDescription * description;
	uint32_t size_bytes;

	template<typename BufferType>
	DataBuffer(BufferData<BufferType> * src_buffer) 
		: data{src_buffer}, description{src_buffer}, size_bytes{src_buffer->size_bytes} {};

};

// Stores cache of device buffer arguments
// TODO: separate host & device resource instances: host has host data, this should be accessible from device
// , thus resource handles = {host handle, device handle}, host and device can be handled separately, & resource 
// id = path 
struct VertexBuffer {
	void * data;
	VertexDataDescription * description;
	uint32_t size;
	uint32_t size_bytes;

	DeviceVertexBuffer * device_data = nullptr;

	template<typename VertexType>
	VertexBuffer(VertexData<VertexType> * vertex_data) 
		: data{vertex_data->data()}, description{vertex_data}
			, size{static_cast<uint32_t>(vertex_data->size())}, size_bytes{vertex_data->sizeBytes()} {};

	VertexBuffer()
		: data{nullptr}, description{nullptr}, size{0}, size_bytes{0} {};

	void load(const ComputeDevice * device, VkBufferUsageFlagBits usage) {
		if(device_data != nullptr) return;

		device_data = new DeviceVertexBuffer;
		device->createDeviceBuffer(device->graphics_queue, data, size_bytes
				, device_data->buffer, device_data->memory, usage);
	}
	void unload() {
		delete device_data;
	}
};

// Triangle-based mesh
struct Mesh {
	VertexBuffer vertices;
	VertexBuffer indices;

	template<typename VertexType, typename IndexType>
	Mesh(VertexData<VertexType> * vertex_data, VertexData<IndexType> * index_data = nullptr) 
		: vertices{VertexBuffer(vertex_data)}, indices{index_data == nullptr? VertexBuffer():VertexBuffer(index_data)} {};

	void load(const ComputeDevice * device) {
		std::cout << "loading vertex data" << std::endl;
		if(vertices.size_bytes) vertices.load(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		std::cout << "loading index data" << std::endl;
		if(indices.size_bytes) indices.load(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		std::cout << "mesh loaded" << std::endl;
	}
};


struct SceneObject {
	Mesh * mesh;
	Material * material;
	// resources_layout (descriptor layout metadata)
};

}
