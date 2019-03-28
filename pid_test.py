#!/usr/bin/env python3
from simple_pid import PID
from math import floor
import time
from time import sleep
fall_rate = 0
def motor(speed):
   global fall_rate
   fall_rate += speed*0.01
set_rate = 100
pid = PID(20,11,1,set_rate)
mark = 0
start_time = time.time()
while 1:
    control = pid(fall_rate)
    control = floor(control*255/1000)
    print('Motor input: ' + str(control))
    motor(control)
    print('fall rate: ' + str(fall_rate))
    sleep(0.05)
    if fall_rate > 99.5 and fall_rate < 100.5:
        mark +=1
    else:
        mark = 0
    if mark > 10:
        break
e_time = time.time()-start_time
print(e_time)



