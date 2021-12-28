#include <iostream>
#include "Simulations.hpp"


using namespace std;

int asd[] = {1, 2, 3, 4, 5,6 ,7 ,8, 9, 10};

void print_array(int * array, int lenght) {
	for(int i =0; i < lenght; i++)
		cout << array[i] << " - ";
}

// render - space hierarchy
//
//

// render triggered from rendering list, or singletime from external call
// rendering cant be cached, cache implies spacedata close coupling 
// => redering call should be from space object
//

// userland and internal object data / behaviour relationships:
// 
// Spacehierarchy should not know details of userland, only entry point trigger
// (void* fn() ?)


// visor = rendering wrapper/context ?

int main(int argc, char * argv[]) {

	cout << "Hello, World!" << endl; // This prints "Hello, World!"
	cout << "MVPMatrix" << endl; // This prints "Hello, World!"
	// for(int i = 0; i < 10; i++) {
		// print_array(asd, 10);
		// cout << asd[i] << endl;
	// }
	
	Simulation * simulation = Simulations::generic();

	simulation->start();

	return 0;
}

