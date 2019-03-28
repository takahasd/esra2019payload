#!/usr/bin/env python3
import numpy
import cv2 as cv
import os
import urllib.request
face_cascade = cv.CascadeClassifier('./examples/data/cascade.xml');
img = cv.imread('sage.jpg');
gray = cv.cvtColor(img,cv.COLOR_BGR2GRAY);
faces = face_cascade.detectMultiScale(gray,1.3,5);
for(x,y,w,h) in faces:
    cv.rectangle(img,(x,y),(x+w,y+h),(255,0,0),2);
    roi_gray = gray[y:y+h, x:x+w];
    roi_color = img[y:y+h, x:x+w];
cv.imshow('img',img);
cv.waitKey(0);
cv.destroyAllWindows();
exit();


