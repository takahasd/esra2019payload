# esra2019payload
***OUTDATED******************************************************************************************************************************
Main repository for the ESRA 2019 payload sub team.
The main components of this repository are the sobel.py and variance.py Python scripts. 
variance.py takes a texture sample and represents it as a sphere in 3D space. The data for the sphere is written into variance.txt. 
sobel.py creates a sobel image of the image being analyzed, and then uses the data from variance script to attempt to identify textures. 
You need the Pillow, math, and os libraries. 
All images need to be in the same directory as the scripts. 
variance.txt needs to be in the same directory as the scripts. 
***OUTDATED******************************************************************************************************************************
c++ works now. "compile" is a bash script that links all the libraries you need. 
works basically the same as the python script. you need a functional openCV install. 
all the parachute control and righting scripts are in the "controls" folder. 
These won't actually run on anything other than the Pi, mostly because your laptop doesnt have GPIO pins. 
The "comms" folder has all the IPC code for communicating between the CV process and the parachute control python process.
I'm 1050 miles from the pi right now, so the C clientside functions will be integrated into the main program when I get back to school.
Serverside python stuff works, and should be able to be imported and run on a separate thread in lander.py, but again, can't run that until I'm with the pi. 
Next up is fixing the pi's openCV install, since the SD card decided to become corrupt. 
Currently running a kernel from early 2017, but hopefully that won't make a difference. 
It probably will. 
