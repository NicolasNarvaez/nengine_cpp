#pragma once
#include <vector>
#include <algorithm>

namespace nengine {
	namespace util {

		namespace vector {
			// template <class T>
			// T merge(T * a, T * b);

			template <class T>
			std::vector<T> * merge(
					  std::vector<T> * a
					, std::vector<std::vector<T>*>* vectors
				) {
				auto total_size {a->size()};
				for(std::vector<T>* single_vector : * vectors)
					total_size += single_vector->size();
					
				std::vector<T>* result;
				result->reserve(total_size);
				for(std::vector<T>* single_vector : * vectors)
					result->insert(result->end()
						, single_vector->begin()
						, single_vector->end());

				return result;
			}
		}

	}
}

