#pragma once
#include <vector>
#include "NObject.hpp"
#include "SpaceTree.hpp"
#include "Visor.hpp"
#include "RenderQuery.hpp"

namespace NEngine {

// class VisorContext {
// };

class SpaceNode {
	// computing context buffers
	public:
	SpaceTree * tree;
	std::vector<NObject*> * objects;
	SpaceNode * parent;
	std::vector<SpaceNode*>* sibblings;
	std::vector<SpaceNode*>* childs;
	// Constructs rendering query from visor definition/spatial graphic features
	RenderQuery * queryRender(Visor * visor, void * context = 0, bool generate_context = false);
	std::vector<NObject*> * allObjects();
};

}
