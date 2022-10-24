#pragma once
#include "LLRenderer.hpp"

namespace NEngine {

class VulkanLLRenderer : public LLRenderer{
	void createGraphicsContext();
	void createGraphicsPipeline();

	public:
	VulkanLLRenderer();
	~VulkanLLRenderer();
	void setSceneGraph(LLSceneGraph * scene);
	void checkSceneGraph();
	void render(RenderQuery * query = nullptr);
};

}
