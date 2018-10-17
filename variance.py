#!/usr/bin/python
from PIL import Image
import math
from os import system
import os
cwd = os.getcwd();
print(cwd);
path = str(input("Sample File Path?\n"));
sample = Image.open(path);
sample = sample.convert('RGB');
width,height = sample.size;
avg_red = 0;
avg_blue = 0;
avg_green = 0;
variance_sum = 0;
variance_max = 0;
for x in range(1,width):
    for y in range(1,height):
        p = sample.getpixel((x,y));
        avg_red += p[0];
        avg_green += p[1];
        avg_blue += p[2];
avg_red = avg_red/(width*height);
avg_green = avg_green/(width*height);
avg_blue = avg_blue/(width*height);
print("Image is centered at: R"+str(int(avg_red))+" G"+str(int(avg_green))+" B"+str(int(avg_blue)));
for x in range(1,width):
    for y in range(1,height):
        p = sample.getpixel((x,y));
        dist_R = avg_red - p[0];
        dist_G = avg_green - p[1];
        dist_B = avg_blue - p[2];
        variance = math.sqrt((dist_R*dist_R) + (dist_G*dist_G) + (dist_B*dist_B));
        if variance > variance_max:
            variance_max = variance;
        variance_sum +=variance;
variance_avg = variance_sum/(width*height);
print("Average variance is " + str(int(variance_avg)));
print("Maximum variance is " + str(int(variance_max)));
f = open("variance.txt","w");
f.write(str(int(avg_red)) + "\n" + str(int(avg_green)) + "\n" + str(int(avg_blue)) + "\n" + str(int(variance_avg)) + "\n" + str(int(variance_max)));
f.close();
