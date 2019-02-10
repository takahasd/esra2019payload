#!/usr/bin/env python3
from RPi import GPIO
import time
import sys
GPIO.setmode(GPIO.BOARD)
GPIO.setup(11,GPIO.OUT)
pwm = GPIO.PWM(11,100)
forward = 13
backward = 15
GPIO.setup(forward,GPIO.OUT)
GPIO.setup(backward,GPIO.OUT)
GPIO.output(forward,GPIO.HIGH)
GPIO.output(backward,GPIO.LOW)
pwm.start(0)
def reel_in(speed):
    GPIO.output(forward,GPIO.HIGH)
    GPIO.output(backward,GPIO.LOW)
    pwm.ChangeDutyCycle(speed)
def reel_out(speed):
    GPIO.output(backward,GPIO.HIGH)
    GPIO.output(forward,GPIO.LOW)
    pwm.ChangeDutyCycle(speed)
while 1:
    com = input("waiting on user...")
    s = com.split()
    try:
        if len(s)>1:
            speed = int(s[1])
        command = s[0]
    except:
        print("nice typo, stupid.")
    if command=='in':
        reel_in(speed)
    if command=='out':
        reel_out(speed)
    if command=='stop':
        reel_in(0)
