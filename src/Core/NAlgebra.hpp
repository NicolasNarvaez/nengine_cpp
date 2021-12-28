#pragma once

namespace nalgebra {
	namespace nvec {
		float * vec(int dim);
		float * op(int dim, char op, float * vec_a, float * vec_b);
	}
	namespace nmat {
		float * mat(int dim);
		float * op(int dim, char op, float * mat_a, float * mat_b);
	}
}
