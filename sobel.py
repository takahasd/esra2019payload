#!/usr/bin/python3.5
import math
from PIL import Image
import os
from os import system
cwd = os.getcwd()
print(cwd);
path = str(input("File Path?"));
save = str(input("Save Sobel image as?"));
img = Image.open(path);
img = img.convert('RGB');
max_intensity = 0;
width, height = img.size;
newimg = Image.new("RGB", (width, height), "white")
#filter only green
#red = 0;
#blue = 0;
for x in range(1, width-1):
	progress = float(x/width);
	if x % 50 == 0:
		print(str(x) + " rows complete...");
	for y in range(1, height-1):
		gradient_x = 0
		gradient_y = 0
		p = img.getpixel((x-1, y-1))
		red = p[0]
		green = p[1]
		blue = p[2]
		intensity = red + green + blue
		gradient_x += -intensity
		gradient_y += -intensity
		p = img.getpixel((x-1, y))
		red = p[0]
		green = p[1]
		blue = p[2]
		gradient_x += -2 * (red + green + blue)
		p = img.getpixel((x-1, y+1))
		red = p[0]
		green = p[1]
		blue = p[2]
		gradient_x += -(red + green + blue)
		gradient_y += (red + green + blue)
		p = img.getpixel((x, y-1))
		red = p[0]
		green = p[1]
		blue = p[2]
		gradient_y += -2 * (red + green + blue)
		p = img.getpixel((x, y+1))
		red = p[0]
		green = p[1]
		blue = p[2]
		gradient_y += 2 * (red + green + blue)
		p = img.getpixel((x+1, y-1))
		red = p[0]
		green = p[1]
		blue = p[2]
		gradient_x += (red + green + blue)
		gradient_y += -(red + green + blue)
		p = img.getpixel((x+1, y))
		red = p[0]
		green = p[1]
		blue = p[2]
		gradient_x += 2 * (red + green + blue)
		p = img.getpixel((x+1, y+1))
		red = p[0]
		green = p[1]
		blue = p[2]
		gradient_x += (red + green + blue)
		gradient_y += (red + green + blue)
		length = math.sqrt((gradient_x * gradient_x) + (gradient_y * gradient_y))
		length = length / 4328 * 255
		length = int(length)
		if length > max_intensity:
			max_intensity = length;
		newimg.putpixel((x,y),(length,length,length))
incrementx = newimg.width/100;
incrementy = newimg.height/100;
gridx = [];
gridy = [];
gridx.append(0);
gridy.append(0);
threshold = 20;
for g in range(1,99):
	gridx.append(gridx[g-1]+incrementx);
	gridy.append(gridy[g-1]+incrementy);
for x in gridx:
	for y in gridy:
		safe = True;
		for i in range(0, int(incrementx)):
			for j in range(0,int(incrementy)):
				p = newimg.getpixel((x+i,y+j));
				if(p[1] > threshold):
					safe = False;
		if safe == True:
			for i in range(0,int(incrementx)):
				for j in range(0,int(incrementy)):
					newimg.putpixel((int(x+i),int(y+j)),(int(255),int(0),int(0)));
newimg.save(save);
print("Maximum intensity is " + str(max_intensity) + ".");
print("Sobel image saved as " + save + " in current directory.");
img.show();
newimg.show();
