#include "NObject.hpp"
#include "NAlgebra.hpp"

using namespace nalgebra;

NObject::NObject() {
	this->pos = nvec::vec(this->dim);
	this->rot = nmat::mat(this->dim);
}
