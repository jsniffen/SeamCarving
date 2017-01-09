/*
* Author: Julian Sniffen
* Description:
*	This file contains the world class, which is used to encapsulate the project and simplify the main function.
*/

#ifndef WORLD_H
#define WORLD_H

#define cimg_display 0

#include <string>
#include "slVector.H"
#include "CImg.h"

using namespace cimg_library;
using namespace std;

class World {
	public:
		//read the image in
		void readImage(string filename);

		//calculate the energy of the pixels
		void calculateEnergy(bool print);

		//updatethe energy of the pixels
		void updateEnergy();

		//remove a seam
		void seamCarve(int desiredWidth, bool print);

		//transpose the image
		void transposeImage();

		//write the image
		void writeImage(string filename);
	private:
		SlVector3 *image;
		double *energy;
		double maxEnergy;
		int currentHeight;
		int currentWidth;
		int height;
		int width;
		int depth;
		int spectrum;
};

#endif
