#include "Visor.hpp"
#include "SpaceNode.hpp"

void Visor::render() {
	RenderQuery * query = this->nobject->nspace->queryRender(this);
	this->renderer(query);
	// send to render every object
};
