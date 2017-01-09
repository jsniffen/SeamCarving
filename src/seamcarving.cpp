/*
* Author: Julian Sniffen
* Description:
*	This file contains the main function for this project.
*/

#include <iostream>
#include <string>

#include "World.h"

using namespace std;

int main(int argc, char* argv[]){
	World world;
	//run the program with the proper arguments
	if(argv[1] && argv[2] && argv[3] && argv[4]){
		world.readImage(argv[1]);
		world.seamCarve(atoi(argv[3]), true);
		world.transposeImage();
		world.seamCarve(atoi(argv[4]), false);
		world.transposeImage();
		world.writeImage(argv[2]);
	}else{
		cout << "Please run the program with the proper arguments" << endl;
		return 0;
	}
}