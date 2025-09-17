import time
from w1thermsensor import W1ThermSensor
import serial
import struct
import binascii
import sys

ser = serial.Serial('/dev/ttyUSB0')
temp_sensor = W1ThermSensor()

startTime =0
nextTime =0
count = 0

startTime = time.time()

ser.baudrate = 115200  # set Baud rate to 115200
ser.bytesize = 8     # Number of data bits = 8
ser.parity   ='N'    # No parity
ser.stopbits = 1     # Number of Stop bits = 1
ser.timeout  = None  # Setting timeouts here No timeouts,waits forever

if ser.is_open:
    print("Serial port opend successfully.")

while True:
    data = 0
    data0 = 0
    data1 = 0
    volt = 0
    num = 2 #センサによって番号を割り当てる

    nextTime = time.time() - startTime

    if nextTime > 5:
        temp = temp_sensor.get_temperature()
        print("水温：{0:.1f}度".format(temp))
        count = count + 1

        data = int(temp * 100)
        data0 = (data >> 8) & 0xFF
        data1 = data & 0xFF

       #先頭を表す文字、センサ種類番号、通し番号、センサデータの上位/下位ビット
        ser.write(bytes([0xAA]))
        ser.write(bytes([num]))
        ser.write(bytes([count]))
        ser.write(bytes([data0]))
        ser.write(bytes([data1]))


        startTime = time.time()
        nextTime = 0

ser.close()
