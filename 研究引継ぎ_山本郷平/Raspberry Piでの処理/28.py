import os
import glob
import time
import subprocess
import serial

# UART設定
ser = serial.Serial('/dev/ttyUSB0', baudrate=115200, bytesize=8, parity='N', stopbits=1, timeout=None)
if ser.is_open:
    print("Serial port opened successfully.")

# 温度センサ設定
os.system('modprobe w1-gpio')
os.system('modprobe w1-therm')

base_dir = '/sys/bus/w1/devices/'
device_folder = glob.glob(base_dir + '28*')[0]
device_file = device_folder + '/w1_slave'

def read_temp_raw():
    catdata = subprocess.Popen(['cat', device_file], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = catdata.communicate()
    out_decode = out.decode('utf-8')
    lines = out_decode.split('\n')
    return lines

def read_temp():
    lines = read_temp_raw()
    while lines[0].strip()[-3:] != 'YES':
        time.sleep(0.2)
        lines = read_temp_raw()
    equals_pos = lines[1].find('t=')
    if equals_pos != -1:
        temp_string = lines[1][equals_pos+2:]
        temp_c = float(temp_string) / 1000.0
        return temp_c

count = 0
while True:
    temp = read_temp()
    print("水温：{0:.1f}度".format(temp))
    count += 1

    # 温度データを100倍して整数化し、2バイトに分割
    data = int(temp * 100)
    data0 = (data >> 8) & 0xFF
    data1 = data & 0xFF

    # UART送信（0xAA:ヘッダー, 0x02:センサの種類, 計測回数, 温度データの上位/下位バイト）
    ser.write(bytes([0xAA]))
    ser.write(bytes([0x02]))
    ser.write(bytes([count]))
    ser.write(bytes([data0]))
    ser.write(bytes([data1]))

    time.sleep(5)
