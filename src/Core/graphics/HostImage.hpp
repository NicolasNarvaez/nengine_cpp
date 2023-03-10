#pragma once

#include "../os/fs.hpp"

#include <cstring>

namespace NEngine {

struct HostImage {
	// path = 0 => ram memory copy
	std::string path;
	void * data = nullptr;

	size_t size_bytes;
	uint32_t width;
	uint32_t height;

	uint32_t channels;
	Format format;

	// TODO: add format auto-detector
	HostImage(Format format, const std::string path) : format{format}, path{path} {};
	// copy from memory
	HostImage(Format format, const void * src_data, size_t size_bytes, uint32_t width, uint32_t height) 
		: format{format}, path{""}, size_bytes{size_bytes}, width{width}, height{height} {
			data = malloc(size_bytes);
			memcpy(data, src_data, size_bytes);
		};

	void load() {
		if(data != nullptr) return;
		fs::loadImage(format, path, data, width, height, channels, size_bytes);
	};
	void unload() {
		if(data == nullptr) return;
		fs::unloadImage(data);
		data = nullptr;
	};
};

}
