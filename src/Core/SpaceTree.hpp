#pragma once
#include <vector>
#include "NObject.hpp"

namespace NEngine {
class SpaceNode;

class SpaceTree {
	public:
	SpaceNode * root;

	std::vector<NObject*>* allObjects();
};
}
