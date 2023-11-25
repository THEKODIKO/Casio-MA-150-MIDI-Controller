import json
import time
from urllib3 import request


import serial




with open("config.json") as file:
    config= json.load(file)


# Open the serial ports.
read_serial = serial.Serial(config["readPort"], baudrate= config["readBaudrate"], timeout= config["timeout"])
write_serial = serial.Serial(config["writePort"], baudrate= config["writeBaudrate"], timeout= config["timeout"])



while 1:
    data= read_serial.read_until(size= config["readBytes"])
    
    write_serial.write(data)
