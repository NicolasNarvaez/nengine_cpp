#include "Visor.hpp"
#include "SpaceNode.hpp"

void Visor::render() {
	RenderQuery * query = this->nobject->nspace->queryRender(this);
	this->renderer->render(query);
	// send to render every object
};
