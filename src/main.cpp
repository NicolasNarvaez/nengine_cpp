#include <iostream>
#include "Core/Simulations.hpp"

using namespace std;

int asd[] = {1, 2, 3, 4, 5,6 ,7 ,8, 9, 10};

void print_array(int * array, int lenght) {
	for(int i =0; i < lenght; i++)
		cout << array[i] << " - ";
}

int main(int argc, char * argv[]) {

	cout << "Hello, World!" << endl; // This prints "Hello, World!"
	cout << "MVPMatrix" << endl; // This prints "Hello, World!"
	cout << "MVPMatrix" << endl; // This prints "Hello, World!"
	// for(int i = 0; i < 10; i++) {
		// print_array(asd, 10);
		// cout << asd[i] << endl;
	// }
	
	NEngine::Simulation * simulation = NEngine::generic();

	simulation->start();

	return 0;
}

