#!/usr/bin/env python3
import os
import cv2 as cv
import numpy
for file_type in ['examples']:
    for img in os.listdir(file_type):
        if file_type == 'Positives':
            line = file_type+'/'+img+'1 0 0 50 50\n';
            with open('info.dat','a') as f:
                f.write(line);
        elif file_type =='Negatives':
            line = file_type+'/'+img+'\n';
            with open('bg.txt','a') as f:
                f.write(line);
