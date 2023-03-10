#pragma once

#include <algorithm>
#include <bitset>
#include <iostream>
#include <string>
#include <vector>

namespace NEngine {
	namespace util {
		// Retrieves & outputs string with binary representation of data.
		static std::string stringBinary(const void* data, uint32_t size_bytes, bool print = true) {
			std::string out_string;
			const char* src_data = reinterpret_cast<const char*>(data);

			for(uint32_t i = 0; i < size_bytes; i++) {
				out_string += std::bitset<8>(src_data[i]).to_string();
				out_string += ", ";
			}

			std::cout << "binary string: " << out_string << std::endl;
			return out_string;
		}


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

