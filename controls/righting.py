#!/usr/bin/env python3
from smbus2 import SMBus
import time
import numpy as np
from RPi import GPIO
bus = SMBus(1)
GPIO.setmode(GPIO.BOARD)
GPIO.setup(16,GPIO.OUT)
GPIO.setup(18,GPIO.OUT)
GPIO.setup(22,GPIO.OUT)
address = 0x68
bus.write_byte_data(address,0x1c,0b00000000)
def pump_on():
    GPIO.output(16,GPIO.HIGH)
    GPIO.output(18,GPIO.LOW)
    GPIO.output(22,GPIO.HIGH)
def pump_off():
    GPIO.output(16,GPIO.LOW)
    GPIO.output(18,GPIO.LOW)
    GPIO.output(22,GPIO.LOW)
def accel():
    x0 = bus.read_byte_data(address,0x3B)
    x1 = bus.read_byte_data(address,0x3C)
    y0 = bus.read_byte_data(address,0x3D)
    y1 = bus.read_byte_data(address,0x3E)
    z0 = bus.read_byte_data(address,0x3F)
    z1 = bus.read_byte_data(address,0x40)
    xval = np.int16((x0 << 8) + x1)
    yval = np.int16((y0 << 8) + y1)
    zval = np.int16((z0 << 8) + z1)
    return ((xval/16384),(yval/16384),(zval/16384))
while True:
    if accel()[2] > -0.8:
        pump_on()
        print("Not upright, pumps are on")
    else:
        pump_off()
        print("Upright, pumps are off")
    time.sleep(0.5)
    

