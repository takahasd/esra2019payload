# esra2019payload
Main repository for the ESRA 2019 payload sub team.
The main components of this repository are the sobel.py and variance.py Python scripts. 
variance.py takes a texture sample and represents it as a sphere in 3D space. The data for the sphere is written into variance.txt. 
sobel.py creates a sobel image of the image being analyzed, and then uses the data from variance script to attempt to identify textures. 
You need the Pillow, math, and os libraries. 
All images need to be in the same directory as the scripts. 
variance.txt needs to be in the same directory as the scripts. 
