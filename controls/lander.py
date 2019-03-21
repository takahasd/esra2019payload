#!/usr/bin/env python3
from RPi import GPIO
import time
import sys
from smbus2 import SMBus
from simple_pid import PID
from math import floor
address = 0x62
bus = SMBus(1)
set_rate = 0
pid = PID(10,0,0,set_rate)
pid.output_limits = (-100,100)
def distance():
    bus.write_byte_data(address,0,4)
    bus.write_byte(address,1)
    while bus.read_byte(address) & 1 !=0:
        pass
    bus.write_byte(address,0xf)
    d = bus.read_byte(address)<<8
    bus.write_byte(address,0x10)
    d|=bus.read_byte(address)
    return d
def speed():
    d0 = distance()
    t0 = time.time()
    time.sleep(0.1)
    d1 = distance()
    t1 = time.time()
    speed = (d0-d1)/((t1-t0))
    if abs(d1-d0)==1:
        speed = 0
    return speed
GPIO.setmode(GPIO.BOARD)
GPIO.setup(11,GPIO.OUT)
pwm = GPIO.PWM(11,100)
forward = 15
backward = 13
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
    vel = speed()
    control = pid(vel)
    print("Fall Rate:" + str(vel))
    print("Motor Input:" + str(control))
    if control > 0:
        reel_in(abs(control))
    else:
        reel_out(abs(control))
    time.sleep(0.1)
#while 1:
  #  com = input("waiting on user...\n")
   # s = com.split()
    #try:
     #   if len(s)>1:
      #      speed = int(s[1])
        #command = s[0]
    #except:
     #   print("nice typo, stupid.")
    #if command=='in':
    #    reel_in(speed)
    #if command=='out':
     #   reel_out(speed)
    #if command=='stop':
     #   reel_in(0)
