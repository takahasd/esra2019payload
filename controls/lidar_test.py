#!/usr/bin/env python3
from smbus2 import SMBus
from smbus2 import SMBusWrapper
import time
bus = SMBus(1)
address = 0x62
def measure():
    bus.write_byte_data(address,0,4)
    bus.write_byte(address,1)
    while bus.read_byte(address) & 1 !=0:
        pass
    bus.write_byte(address,0xf)
    d = bus.read_byte(address)<<8
    bus.write_byte(address,0x10)
    d|=bus.read_byte(address)
    return d
while True:
    d = measure()
    print(d)
    time.sleep(0.5)



