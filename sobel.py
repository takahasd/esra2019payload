#!/usr/bin/python3.5
#***********************************************
#Program: sobel.py
#Author: takahashi for ESRA payload
#Date: 10/10/2018
#***********************************************
import math
import time
from PIL import Image
import os
from os import system
cwd = os.getcwd();
os.system('python3 variance.py');#run the other script. read it if you haven't.
print(cwd);#this is the directory you need to put the images in
path = str(input("Main File Path?\n"));
#save = str(input("Save Sobel image as?\n"));
rewidth = 426;#dimensions for resizing. large images take too long.
hsize = 240;
bushes = Image.open(path); #open the image o be sobel-ed
bushes = bushes.resize((rewidth,hsize),Image.ANTIALIAS);#resize image. I don't need this 4K bullshit
bushes.save("COPY.jpg");#save a copy, we need two of these.
bushes = bushes.convert('RGB');#make sure it's the right type; without this line, some .png files fail
max_intensity = 0; #keep track of maximum value for scaling purposes
width, height = bushes.size; #get size of image
sobel = Image.new("RGB", (width, height), "white");#create a new blank image of the same dimensions
greenscale = Image.new("RGB", (width, height),"white"); #create a new blank imagefor each color scale image
redscale = Image.new("RGB",(width,height),"white");#obsolete
bluescale = Image.new("RGB",(width,height),"white");
variance_map = Image.new("RGB",(width,height),"white");
#filter only green
#red = 0;
#blue = 0;
#for x in range(1, width-1): #iterating through each pixel in the image 
#    progress = float(x/width);
#    if x % 50 == 0:
#        print(str(x) + " rows complete...");# display progress every 50 rows
#    for y in range(1, height-1):
#        gradient_x = 0;#begin sobel operator
#        gradient_y = 0;
#        p = bushes.getpixel((x-1, y-1))#this operation is done for each surrounding pixel
#        red = p[0];
#        green = p[1];#get each color intensity
#        blue = p[2];
#        gradient_x += -(red + green + blue);#look up sobel algorithm matrices to understand why this is happening
#        gradient_y += -(red + green + blue);
#        p = bushes.getpixel((x-1, y));#next neighbor pixel
#        red = p[0];
#        green = p[1];#more intensities
#        blue = p[2];
#        gradient_x += -2 * (red + green + blue);#refer to sobel matrix
#        p = bushes.getpixel((x-1, y+1));#next neighbor pixel
#        red = p[0];
#        green = p[1];#more intensities
#        blue = p[2];
#        gradient_x += -(red + green + blue);#more sobel matrices
#        gradient_y += (red + green + blue);
#        p = bushes.getpixel((x, y-1));#more neighbors
#        red = p[0];
#        green = p[1];#more intensities
#        blue = p[2];
#        gradient_y += -2 * (red + green + blue);#more sobel
#        p = bushes.getpixel((x, y+1));#more neighbors
#        red = p[0];
#        green = p[1];#more intensities
#        blue = p[2];
#        gradient_y += 2 * (red + green + blue);#more sobel
#        p = bushes.getpixel((x+1, y-1));#more neighbors
#        red = p[0];
#        green = p[1];#more intensities
#        blue = p[2];
#        gradient_x += (red + green + blue);
#        gradient_y += -(red + green + blue);#more sobel
#        p = bushes.getpixel((x+1, y));#more neighbors
#        red = p[0];
#        green = p[1];#more intensities
#        blue = p[2];
#        gradient_x += 2 * (red + green + blue);#more sobel
#        p = bushes.getpixel((x+1, y+1));#last neighbor
#        red = p[0];
#        green = p[1];#last intensities
#        blue = p[2];
#        gradient_x += (red + green + blue);#last sobel
#        gradient_y += (red + green + blue);
#        length = math.sqrt((gradient_x * gradient_x) + (gradient_y * gradient_y));#we previously had an x and y gradient, now we want the total length of the gradient
#        length = length / 4328 * 255;#we need to scale this value to 255 in order to make new pixels from it
#        length = int(length); #floats r bad
#        if length > max_intensity:
#            max_intensity = length;#keep track of the mmax intensity pixel
#        sobel.putpixel((x,y),(length,length,length));#place a pixel with an intensity equal to the length of the gradient
#        p = bushes.getpixel((x,y));#this is for making color scale images; get a pixel
#        red = p[0];
#        green = p[1];#get intensities
#        blue = p[2];
#        greenscale.putpixel((x,y),(0,green,0));
#        redscale.putpixel((x,y),(red,0,0));  #write each color to an individual image
#        bluescale.putpixel((x,y),(0,0,blue));
gridsize = 100;
incrementx = sobel.width/gridsize;#creating a grid of discrete sites
incrementy = sobel.height/gridsize;
gridx = [];#initializing grid starting point arrays
gridy = [];
gridx.append(0);#zero elements
gridy.append(0);
threshold = 30;#intensity threshold for recognizing values, set to 255 to ignore sobel
bushid = Image.open("COPY.jpg");
bushid.save("COPY2.jpg");
compliance = Image.open("COPY2.jpg");
bushid = bushid.convert('RGB');
greenmax = 100;
print("Sobel imaging complete.");
f = open("variance.txt","r");#opening the variance file to read values
center_red = f.readline();#read in values of the center of variance
center_green = f.readline();
center_blue = f.readline();
variance_avg = f.readline();#average variance of the sample texture
variance_max = f.readline();#maximum variance of the sample texture
f.close();
print("Reading variance data...");
print("Target center is R" + str(int(center_red)) + " G" + str(int(center_green))+ " B"+str(int(center_blue))+".");
print("Average variance is " + str(int(variance_avg)) + ".");
print("Maximum variance is " + str(int(variance_max)) + ".");
print("Establishing grid...");
g = 0;
while width > gridx[-1] + 2*int(incrementx):#make the grid. not worth explaining
    g+=1;
    gridx.append(math.floor(gridx[g-1]+int(incrementx)));#For each grid space, initialize x and y values
g = 0;
gridcount = 0;#grid count for user
while height > gridy[-1] + 2*int(incrementy):
    g+=1;
    gridy.append(math.floor(gridy[g-1]+int(incrementy)));
print("Beginning edge and variance comparison...");
for x in gridx:#iterating through each grid
    for y in gridy:
        gridcount +=1;
        bush = True;
        gridComplianceCount = 0;#how many pixels arewithin the average variance of the texture
        for i in range(0, int(incrementx)):#iterating through each pixel in each grid
            for j in range(0,int(incrementy)):
#                p = sobel.getpixel((x+i,y+j));
                b = bushes.getpixel((x+i,y+j));
                variance = math.sqrt((int(center_red)-b[0])*(int(center_red)-b[0]) + (int(center_green) - b[1])*(int(center_green)-b[1]) + (int(center_blue)-b[2])*(int(center_blue)-b[2]));#calculate variance.
                if variance < int(variance_avg):#if it complies, say it complies.
                    gridComplianceCount += 1;
                variance_map.putpixel((int(x+i),int(y+j)),(int(variance),int(variance),int(variance)));#draw it on the variance image. for developer
                #removed conditional was if p[1] > threshold
                if(variance > int(variance_max)):#if the pixel is too intense or if theres too much green intensity in the original image
                    bush = False;
        gridCompliance = int(100*float(gridComplianceCount/24));#calculate variance compliance percentage
        if(gridCompliance < 2):#if the grid has <2% grid compliance, its probably not a bush. 
            bush = False;
        if bush == True:
            for i in range(0,int(incrementx)):#iterate through each pixel again to color them
                for j in range(0,int(incrementy)):
                    bushid.putpixel((int(x+i),int(y+j)),(255,0,0));#color grid red
        for i in range(0,int(incrementx)):
                for j in range(0,int(incrementy)):
                        compliance.putpixel((int(x+i),int(y+j)),(int(255*float(gridCompliance)/100),int(255*float(gridCompliance)/100),int(255*float(gridCompliance)/100)));#draw compliance percentages.
    print("Completed " + str(gridcount) + " grids out of " + str(len(gridx)*len(gridy)));
print("Finished.");#finir
#sobel.save(save);
#greenscale.save("greenscale.jpg");#save all images
#bluescale.save("bluescale.jpg");
#redscale.save("redscale.jpg");
bushid.save("bushid.jpg");
#print("Maximum intensity in sobel image is " + str(max_intensity) + ".");#some good info to have
#print("Sobel image saved as " + save + " in current directory.");
bushid.show();
#sobel.show();
variance_map.show();
variance_map.save("variance.jpg");
compliance.show();
compliance.save("compliance.jpg");
print(len(gridx));
print(gridx);
print(incrementx);
print(len(gridy));
print(gridy);
print(incrementy);
#greenscale.show();#show all images
#redscale.show();
#bluescale.show();
