#pragma once
#include <vector>
#include "NObject.hpp"
class SpaceNode;

class SpaceTree {
	public:
	SpaceNode * root;

	std::vector<NObject*>* allObjects();
};
