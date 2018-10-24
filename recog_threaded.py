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
import threading
cwd = os.getcwd();
os.system('python3 variance.py');#run the other script. read it if you haven't.
print("Place images in " + cwd);#this is the directory you need to put the images in
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
gridsize = 100;
incrementx = sobel.width/gridsize;#creating a grid of discrete sites
incrementy = sobel.height/gridsize;
gridx = [];#initializing grid starting point arrays
gridy = [];
gridx.append(0);#zero elements
gridy.append(0);
threshold = 30;#intensity threshold for recognizing values, set to 255 to ignore sobel
greenmax = 100;
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
blank1 = Image.new("RGB",(426,240));#blank images to fill the global image array with.
blank2 = Image.new("RGB",(426,240));
blank3 = Image.new("RGB",(426,240));
blank4 = Image.new("RGB",(426,240));
images= [];#blank image array. Getting the values returned by individual threads is hard, so they just append the return to a global list.
images.append(blank1);
images.append(blank2);#filling the array with blank images for now
images.append(blank3);
images.append(blank4);
def stitch(ul,ur,ll,lr):#stitches together the individual thread images back into the original
    stitched = Image.new("RGB",(426,240));
    stitched.paste(ul,(0,0));
    stitched.paste(ur,(213,0));
    stitched.paste(ll,(0,120));
    stitched.paste(lr,(213,120));
    return stitched;
def featureRecog(image,x1,y1,x2,y2,tn):#This function is run by each thread. 
    print("Thread " + str(tn) + " starting...");
    gridsize = 25;
    area = (x1,y1,x2,y2);#crop area; one quadrant
    section = image.crop(area);
    section.save("CROP_" + str(x1) + "_" + str(y1) + ".jpg");#making copies for other images
    gridx = [];#create grids
    gridy = [];
    gridx.append(0);
    gridy.append(0);
    gridcount = 0;
    bushid = Image.open("CROP_" + str(x1) + "_" + str(y1) + ".jpg");
    bushid.save("COMP_" + str(x1) + "_" + str(y1) + ".jpg");
    width = section.width;
    height = section.height;
    compliance = Image.new("RGB",(width,height));
    g = 0;
    while width > gridx[-1] + 2*int(incrementx):#fill grid lists
        g+=1;
        gridx.append(math.floor(gridx[g-1]+int(incrementx)));
    g = 0;
    while height > gridy[-1] + 2*int(incrementy):
        g+=1;
        gridy.append(math.floor(gridy[g-1]+int(incrementy)));
    #print("Beginning edge and variance comparison...");
    for x in gridx:#iterating through each grid
        for y in gridy:
            gridcount +=1;
            bush = True;
            gridComplianceCount = 0;#how many pixels arewithin the average variance of the texture
            for i in range(0, int(incrementx)):#iterating through each pixel in each grid
                for j in range(0,int(incrementy)):
#                   p = sobel.getpixel((x+i,y+j));
                    #print(x+i);
                    #print(y+j);
                    b = section.getpixel((x+i,y+j));
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
        #print("Completed " + str(gridcount) + " grids out of " + str(len(gridx)*len(gridy)));   
    #bushid.show();
    #print(gridx);
    #print(int(incrementx));
    #print(gridy);
    #print(int(incrementy));
    if x1==0 and y1==0:#depending on what thread you are, put your image in the global list
        images[0] = bushid;
    if x1==212 and y1==0:
        images[1] = bushid;
    if x1==0 and y1==115:
        images[2] = bushid;
    if x1==212 and y1==115:
        images[3] = bushid;
    print("Thread " + str(tn) +" finishing...");
threads = [];#thread list
t1 = threading.Thread(target=featureRecog,args=(bushes,0,0,212,120,1));#create and start threads
t1.start();
threads.append(t1);
t2 = threading.Thread(target=featureRecog,args=(bushes,212,0,421,120,2));
t2.start();
threads.append(t2);
t3 = threading.Thread(target=featureRecog,args=(bushes,0,115,212,235,3));
t3.start();
threads.append(t3);
t4 = threading.Thread(target=featureRecog,args=(bushes,212,115,421,235,4));
t4.start();
for thread in threads:
    thread.join();#join all the threads after they finish
complete = stitch(images[0],images[1],images[2],images[3]);#stitch the quadrants back into the main image
complete.show();#show the completed image.
print("Finished.");#finir
#sobel.save(save);
#greenscale.save("greenscale.jpg");#save all images
#bluescale.save("bluescale.jpg");
#redscale.save("redscale.jpg");
#bushid.save("bushid.jpg");
#print("Maximum intensity in sobel image is " + str(max_intensity) + ".");#some good info to have
#print("Sobel image saved as " + save + " in current directory.");
#bushid.show();
#sobel.show();
#variance_map.show();
#variance_map.save("variance.jpg");
#compliance.show();
#compliance.save("compliance.jpg");
#print(len(gridx));
#print(gridx);
#print(incrementx);
#print(len(gridy));
#print(gridy);
#print(incrementy);
#greenscale.show();#show all images
#redscale.show();
#bluescale.show();
