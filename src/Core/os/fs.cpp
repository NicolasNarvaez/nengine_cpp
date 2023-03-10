#include "fs.hpp"

// #define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <map>


namespace NEngine::fs {

const std::map<FormatId, int> formatSTBIDesiredChannels {
	{FormatId::R8G8B8A8_SRGB, STBI_rgb_alpha}
};

void loadImage(
		  Format format, std::string path
		, void * & data, uint32_t & width, uint32_t & height, uint32_t & channels
		, size_t & size_bytes
) {

	int i_width, i_height, i_channels;
	int desired_channels = formatSTBIDesiredChannels.at(format.format_id);

	stbi_uc* pixels = stbi_load(path.c_str(), &i_width, &i_height, &i_channels, formatSTBIDesiredChannels.at(format.format_id));

	width = i_width; height = i_height; channels = i_channels;
	data = pixels;
	size_bytes = width * height * desired_channels;
}

void unloadImage(void * data) {
		stbi_image_free(data);
}

}
