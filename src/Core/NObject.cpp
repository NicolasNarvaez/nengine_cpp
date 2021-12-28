#include "NObject.hpp"

NObject::NObject() {
	this->pos = nvec::vec(this->dim);
	this->rot = nmat::mat(this->dim);
}
