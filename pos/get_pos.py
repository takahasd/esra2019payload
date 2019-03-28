#!/usr/bin/env python3
import urllib.request
import os
import cv2 as cv
import numpy
import ssl
ssl._create_default_https_context = ssl._create_unverified_context
pos_images_link = 'http://image-net.org/api/text/imagenet.synset.geturls?wnid=n12014085';
pos_image_urls = urllib.request.urlopen(pos_images_link).read().decode();
pic_num = 1;
if not os.path.exists('Positives'):
        os.makedirs('Positives');
for i in pos_image_urls.split('\n'):
    try:
        print(i)
        urllib.request.urlretrieve(i,"Positives/"+str(pic_num)+".jpg");
        img = cv.imread("Positives/"+str(pic_num)+".jpg",cv.IMREAD_GRAYSCALE);
        resized_image = cv.resize(img,(100,100));
        print("Writing image " + str(pic_num) + "\n");
        cv.imwrite("Positives/"+str(pic_num)+".jpg",resized_image);
        pic_num+=1;
    except Exception as e:
        print(str(e));

