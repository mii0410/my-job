import serial
import time

def main():
    try:
        ser = serial.Serial(
            port='COM5',  # デバイスポート番号（自分のポート番号に合わせる）
            baudrate=115200,  # ボーレート
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=1
        )
        if ser.is_open:  # シリアルポートがオープンされているかチェック
            print("Serial port opened successfully.")

        while True:  # データ受信
            if ser.in_waiting > 1:  # 受信バッファにデータがあるか確認
                sensor_num = ord(ser.read(1))

                # 正しいセンサ番号かチェック（1, 2, 3, 4, 5, 6）
                #温度センサは１、水質センサは２など
                if sensor_num not in [1, 2, 3, 4, 5, 6]:
                    continue  # 次のデータへ

                count_data = ord(ser.read(1)) #通し番号
                address = ord(ser.read(1)) #End deviceのshort address
                u8LQI = ord(ser.read(1)) #LQI値
                high_data = ord(ser.read(1)) #センサデータの上位ビット
                low_data = ord(ser.read(1)) #センサデータの下位ビット
                val = (high_data << 8) | low_data #上位ビットと下位ビットで元のセンサデータに戻す
                temperature = val / 100.0 #センサデータを温度に変換する式

                address_str = f"00:{address:02x}"

                u8dbm = (7 * u8LQI - 1970) / 20

                print(f" Sensor {sensor_num} | count: {count_data} | {temperature:.1f}°C | LQI: {u8LQI} | dBm: {u8dbm} | short addr: {address_str}")

                time.sleep(5)  # 5秒待機

    except serial.SerialException as e:
        print("Error opening or operating the serial port:", e)
    finally:
        if ser.is_open:
            ser.close()
            print("Serial port closed.")

if __name__ == "__main__":
    main()
