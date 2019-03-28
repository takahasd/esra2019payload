#!/usr/bin/env python3
import os
import cv2 as cv
import numpy
for img in os.listdir('Negatives'):
    line = 'Negatives'+'/'+img+'\n'
    with open('bg.txt','a') as f:
        f.write(line)
