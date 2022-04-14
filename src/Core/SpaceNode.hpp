#pragma once

#include <vector>
#include "NObject.hpp"
#include "SpaceTree.hpp"
#include "Visor.hpp"

namespace NEngine {

class VisorContext {
};

class RenderQuery {
	public:
	// just intersected objects by now
	std::vector<NObject*>* objects;
	NCamera * camera;
};

class SpaceNode {
	// computing context buffers
	public:
	SpaceTree * tree;
	std::vector<NObject*> * objects;
	SpaceNode * parent;
	SpaceNode * sibblings;
	std::vector<SpaceNode*>* childs;
	RenderQuery * queryRender(Visor * visor, VisorContext * context = 0, bool generate_context = false);
	std::vector<NObject*> * allObjects();
};

}
