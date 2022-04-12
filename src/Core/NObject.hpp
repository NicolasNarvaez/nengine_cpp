#pragma once 
class SpaceNode;

class NObject {
	public:
          int dim = 4;
          float *pos;
          float *rot;
          float *bbox;

          SpaceNode *nspace;
          NObject();
};

class NCamera {
	float dim;
	float * amplitude; 
};
