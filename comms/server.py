#!/usr/bin/env python3
import socket
def vel_server():
    HOST = '127.0.0.1'
    PORT = 65435
    with socket.socket(socket.AF_INET,socket.SOCK_STREAM) as s:
        s.bind((HOST,PORT))
        while True:
            s.listen()
            conn,addr = s.accept()
            with conn:
                print("Connected by",addr)
                while True:
                    data = conn.recv(1024)
                    if not data:
                        break
                    velocity = int.from_bytes(data,byteorder = 'little')
                    print("Recieved setpoint of ",velocity)
                    conn.sendall(b'confirm received')
if __name__=="__main__":
    vel_server()
