#pragma once

#include "../graphics/resources.hpp"

#include <fstream>
#include <vector>

namespace NEngine::fs {

static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if(!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

// size = width*height*layers(format)
void loadImage(
		  Format format, std::string path
		, void * & data, uint32_t & width, uint32_t & height, uint32_t & channels, size_t & size_bytes);

void unloadImage(void * data);

}
