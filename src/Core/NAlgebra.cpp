// small, temporary helper ...
// need to check best alternatives
//

#pragma once


namespace nalgebra {

	namespace nvec {

		float * vec(int dim) {
			return new float[dim];
		}

		// operates over vec_a, changing its values
		float * op(int dim, char op, float * vec_a, float * vec_b) {
			for(int i = 0; i < dim; i++) {
				switch (op) {
					case '*': vec_a[i]*=vec_b[i];
						break;
					case '/': vec_a[i]/=vec_b[i];
						break;
					case '+': vec_a[i]+=vec_b[i];
						break;
					case '-': vec_a[i]-=vec_b[i];
						break;
				}
			}
			return vec_a;
		}
	}

	namespace nmat {
		float * mat(int dim) {
			return new float[dim*dim];
		}

		float * op(int dim, char op, float * mat_a, float * mat_b) {

			return 0;
		}
	}

}
