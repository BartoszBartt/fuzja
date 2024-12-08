import serial
import csv
import re
import os
from datetime import datetime

# Funkcja do wyodrębnienia danych z ciągu
def extract_data(sensor_data):
    pattern = r"AccX: (-?\d+) \| AccY: (-?\d+) \| AccZ: (-?\d+) \| GyroX: (-?\d+) \| GyroY: (-?\d+) \| GyroZ: (-?\d+)"
    match = re.match(pattern, sensor_data)
    
    if match:
        data = {
            "AccX": int(match.group(1)),
            "AccY": int(match.group(2)),
            "AccZ": int(match.group(3)),
            "GyroX": int(match.group(4)),
            "GyroY": int(match.group(5)),
            "GyroZ": int(match.group(6))
        }
        return data
    else:
        return None

# Funkcja do zapisu danych do pliku tekstowego
def save_data_to_txt(data, index):
    filename = f"data_{index}.txt"
    with open(filename, "w") as file:
        for key, value in data.items():
            file.write(f"{key}: {value}\n")
    print(f"Dane zapisane w pliku tekstowym: {filename}")

# Funkcja do zapisu danych do pliku CSV
def save_data_to_csv(data, record_id, current_time):
    filename = "data.csv"
    first_row = ["Id", "Time", "AccX", "AccY", "AccZ", "GyroX", "GyroY", "GyroZ"]
    
    # Sprawdź, czy plik istnieje i czy jest pusty
    file_exists = os.path.isfile(filename)
    file_is_empty = not file_exists or os.path.getsize(filename) == 0

    with open(filename, mode="a", newline="") as file:
        writer = csv.writer(file)
        if file_is_empty:
            # Zapis nagłówków, jeśli plik jest pusty
            writer.writerow(first_row)
        # current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        writer.writerow([record_id, current_time, data["AccX"], data["AccY"], data["AccZ"], data["GyroX"], data["GyroY"], data["GyroZ"]])
    print(f"Dane zapisane w pliku CSV: {filename}")

# Konfiguracja portu szeregowego
port = 'COM5'
baud_rate = 115200

try:
    # Otwórz połączenie szeregowe
    ser = serial.Serial(port, baud_rate, timeout=1)
    print(f"Połączono z {port} przy prędkości {baud_rate}")

    index = 1
    record_id = 1
    # while os.path.exists(f"data_{index}.txt"):
    # index += 1
    current_time = 0
    # Czytaj dane w czasie rzeczywistym
    while True:
        if ser.in_waiting > 0:
            raw_data = ser.readline().decode('utf-8', errors='ignore').strip()
            if raw_data:
                sensor_data = extract_data(raw_data)
                if sensor_data:
                    # save_data_to_txt(sensor_data, index)
                    current_time += 0.005
                    current_time = round(current_time, 4)
                    save_data_to_csv(sensor_data, record_id, current_time)
                    # index += 1
                    record_id += 1
                else:
                    print("Nieprawidłowy format danych:", raw_data)

except serial.SerialException as e:
    print(f"Błąd przy próbie połączenia z {port}: {e}")

except KeyboardInterrupt:
    print("Zakończono odczyt danych.")

finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("Port szeregowy zamknięty.")
