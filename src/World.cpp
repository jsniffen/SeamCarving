/*
* Author: Julian Sniffen
* Description:
*	This file contains the function definitions for the World class
*/

#define cimg_display 0

#include "World.h"
#include "Cost.h"
#include "slVector.H"
#include "CImg.h"

using namespace cimg_library;
using namespace std;

void World::readImage(string filename){
	CImg<double> input(filename.c_str());
	depth = input.depth();
	spectrum = input.spectrum();
	height = input.height();
	width = input.width();
	currentHeight = input.height();
	currentWidth = input.width();

	//create pixel array and energy array
	image = new SlVector3[height*width];
	energy = new double[height*width];

	//convert image to Lab colorspace
	CImg<double> lab = input.RGBtoLab();

	//initialize pixel and energy array
	for(int y = 0; y < height; y++){
		for(int x = 0; x < width; x++){
			energy[y*width + x] = 0;
			image[y*width + x][0] = lab(x, y, 0);
			image[y*width + x][1] = lab(x, y, 1);
			image[y*width + x][2] = lab(x, y, 2);
		}
	}
}

void World::calculateEnergy(bool print){
	//loop through the pixels
	for(int y = 0; y < currentHeight; y++){
		for(int x = 0; x < currentWidth; x++){
			double gX, gY;
			gX = mag(image[y*currentWidth + min((currentWidth - 1), (x + 1))] - image[y*currentWidth + max(0, (x - 1))]);
			gY = mag(image[min((currentHeight - 1), (y + 1))*currentWidth + x] - image[max(0, (y - 1))*currentWidth + x]);

			energy[y*currentWidth + x] = mag(SlVector3(gX, gY, 0));
		}
	}

	//only print the energy the original time
	if(print){
		CImg<double> output(currentWidth, currentHeight, depth, spectrum, 0);

		for(int y = 0; y < output.height(); y++){
			for(int x = 0; x < output.width(); x++){
				output(x, y) = pow(energy[y*output.width() + x]/0.0001, 1.0/3.0);
			}
		}
		
		CImg<double> rgb = output.LabtoRGB();
		rgb.save_jpeg("energy.jpg");
	}

}

void World::updateEnergy(){
	SlVector3 *tempImage  = new SlVector3[currentWidth*currentHeight];
	double *tempEnergy = new double[currentWidth*currentHeight];

	for(int y = 0; y < currentHeight; y++){
		int x_dim = 0;
		for(int x = 0; x < currentWidth; x++){
			while(image[y*width + x_dim][0] == -1){
				x_dim++;
			}
			tempImage[y*currentWidth + x] = image[y*width + x_dim];		
			x_dim++;
		}
	}

	for(int y = 0; y < currentHeight; y++){
		for(int x = 0; x < currentWidth; x++){
			double gX, gY;
			gX = mag(tempImage[y*currentWidth + min((currentWidth - 1), (x + 1))] - tempImage[y*currentWidth + max(0, (x - 1))]);
			gY = mag(tempImage[min((currentHeight - 1), (y + 1))*currentWidth + x] - tempImage[max(0, (y - 1))*currentWidth + x]);

			tempEnergy[y*currentWidth + x] = mag(SlVector3(gX, gY, 0));
		}
	}

	for(int y = 0; y < currentHeight; y++){
		int x_dim = 0;
		for(int x = 0; x < currentWidth; x++){
			while(energy[y*width + x_dim] == -1){
				x_dim++;
			}
			energy[y*width + x_dim] = tempEnergy[y*currentWidth + x];
			x_dim++;
		}
	}

	delete [] tempEnergy;
	delete [] tempImage;
}

void World::seamCarve(int desiredWidth, bool print){
	//calculate the energy and output to energy.jpg
	calculateEnergy(print);

	//new image
	SlVector3 *newImage;

	//while the image is wider than desired remove a vertical seam
	while(desiredWidth < currentWidth){

		//create cost array
		Cost *cost = new Cost[currentHeight*currentWidth];

		//iterate through the energy array calculating costs
		for(int y = 0; y < currentHeight; y++){
			int x_dim = 0;
			for(int x = 0; x < currentWidth; x++){

				while(energy[y*width + x_dim] == -1){
					x_dim++;
				}

				//set the base row for cost array
				cost[y*currentWidth + x].cost = energy[y*width + x_dim];
				cost[y*currentWidth + x].dir = 't';
				cost[y*currentWidth + x].min = false;

				//if we're not on the top row, calculate based off of row above
				if(y != 0){
					if(	
						x != 0 &&
						cost[(y - 1)*currentWidth + (x - 1)].cost < cost[(y - 1)*currentWidth + (x)].cost &&
						(x == (currentWidth - 1) || cost[(y - 1)*currentWidth + (x - 1)].cost < cost[(y - 1)*currentWidth + (x + 1)].cost)
						){
						//left is smallest
						cost[y*currentWidth + x].dir = 'l';
						cost[y*currentWidth + x].cost += cost[(y - 1)*currentWidth + (x - 1)].cost;
					}else if(
						x != (currentWidth - 1) &&
						(x == 0 || cost[(y - 1)*currentWidth + (x + 1)].cost < cost[(y - 1)*currentWidth + (x - 1)].cost) &&
						cost[(y - 1)*currentWidth + (x + 1)].cost < cost[(y - 1)*currentWidth + (x)].cost
						){
						//right is smallest
						cost[y*currentWidth + x].dir = 'r';
						cost[y*currentWidth + x].cost += cost[(y - 1)*currentWidth + (x + 1)].cost;
					}else{
						//middle is smallest
						cost[y*currentWidth + x].dir = 'c';
						cost[y*currentWidth + x].cost += cost[(y - 1)*currentWidth + (x)].cost;
					}
				}

				x_dim++;
			}
		}

		//set the first to min by default
		int index = 0;
		cost[(currentHeight - 1)*currentWidth].min = true;

		//find the minimum cost
		for(int x = 0; x < currentWidth; x++){
			if(cost[(currentHeight - 1)*currentWidth + x].cost < cost[(currentHeight - 1)*currentWidth + index].cost){
				cost[(currentHeight - 1)*currentWidth + index].min = false;
				cost[(currentHeight - 1)*currentWidth + x].min = true;
				index = x;
			}
		}

		//remove the calculated seam
		for(int y = (currentHeight - 1); y > -1; y--){
			int x_dim = 0;
			for(int x = 0; x < (index + 1); x++){
				while(energy[y*width + x_dim] == -1){
					x_dim++;
				}

				//this is the pixel we want to remove
				if(x == index){
					energy[y*width + x_dim] = -1;
					image[y*width + x_dim][0] = -1;
				}

				x_dim++;
			}

			if(cost[y*currentWidth + index].dir == 'l'){
				index--;
			}else if(cost[y*currentWidth + index].dir == 'r'){
				index++;
			}else{
				//do nothing
			}

		}

		//update the energy with seam removal
		updateEnergy();					

		//delete the cost matrix
		delete [] cost;

		//decrement width
		currentWidth--;
	}

	//create the smaller image
	newImage = new SlVector3[currentWidth*currentHeight];

	//copy the old image over, ignoring seam
	for(int y = 0; y < currentHeight; y++){
		int x_dim = 0;
		for(int x = 0; x < currentWidth; x++){
			while(image[y*width + x_dim][0] == -1){
				x_dim++;
			}
			newImage[y*currentWidth + x] = image[y*width + x_dim];		
			x_dim++;
		}
	}

	//delete old image and point to new image
	delete [] image;
	image = newImage;

}

void World::transposeImage(){
	SlVector3 *newImage = new SlVector3[currentWidth*currentHeight];
	for(int y = 0; y < currentHeight; y++){
		for(int x = 0; x < currentWidth; x++){
			newImage[x*currentHeight + y] = image[y*currentWidth + x];
		}
	}	
	delete [] image;
	image = newImage;

	int temp = currentWidth;
	currentWidth = currentHeight;
	currentHeight = temp;

	temp = width;
	width = height;
	height = temp;
}

void World::writeImage(string filename){
	CImg<double> output(currentWidth, currentHeight, depth, spectrum, 0);

	for(int y = 0; y < currentHeight; y++){
		for(int x = 0; x < currentWidth; x++){
			output(x, y, 0) = image[y*currentWidth + x][0];
			output(x, y, 1) = image[y*currentWidth + x][1];
			output(x, y, 2) = image[y*currentWidth + x][2];
		}
	}

	CImg<double> rgb = output.LabtoRGB();
	rgb.save_jpeg(filename.c_str());

	delete [] image;
	delete [] energy;
}