#include <iostream>
#include "Renderer.hpp"
#include "SpaceNode.hpp"

namespace NEngine {

Renderer::Renderer(LLRenderer * ll_renderer_) {
	this->ll_renderer = ll_renderer_;
}
void Renderer::render(RenderQuery * query) {
	// for( object in NObject)
		// compute object MVP
		// render object
	// this->ll_renderer->render(query);
}

void Renderer::run(RenderScene * render_scene) {
	this->ll_renderer->run(render_scene);
}

}
