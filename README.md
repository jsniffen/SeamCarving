# SeamCarving
A smart image resizer based off of a research paper by Shai Avidan and Ariel Shamir.

The progam utilizes the powerful CImg library to read in a .jpg image and convert it
to the LAB colorspace.  Next the program calculates the energy of the image based off
of its gradient.  After calculating the energy, the program calculates a minium cost vertical
seam for the image using dynamic programming and subsequently removes that seam from the image.
After removing the specified amount of vertical seams, the program transposes the image repeats
the process.  After the image is at the desired resolution, the program converts the image back
to the RGB colorspace and outputs it to the specified .jpg file.

# How to Run
```
	'make'	# builds the project
	'make clean'	# removes the executable file 'bin/runner' and the build folder
	'make house'	# runs: bin/runner house.jpg house-out.jpg 312 384
	'make lake'	# runs: bin/runner lake.jpg lake-out.jpg 450 300
	'make pavement'	# runs: bin/runner pavement.jpg pavement-out.jpg 375 200
```

To Run Manually:
```
	'bin/runner in.jpg out.jpg length width'	# generates an out.jpg from the image in.jpg with the resolution length x width 
```
