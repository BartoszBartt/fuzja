#include <Arduino.h>
#include <LSM6DSLSensor.h>
#include <Wire.h>

#define numAxes 3

LSM6DSLSensor *AccGyr; // Akcelerometr i żyroskop

uint32_t previous_tick = 0;
uint32_t current_tick = 0;
char report[256];

void setup() {
  // Inicjalizacja monitora szeregowego
  Serial.begin(115200);
  Wire.begin();

  // Inicjalizacja akcelerometru i żyroskopu
  AccGyr = new LSM6DSLSensor(&Wire);
  AccGyr->begin();
  AccGyr->Enable_X(); // Włączenie akcelerometru
  AccGyr->Enable_G(); // Włączenie żyroskopu

  previous_tick = micros();
}

void loop() {
  // Bufory na dane z akcelerometru i żyroskopu
  int32_t accelData[numAxes];
  int32_t gyroData[numAxes];

  current_tick = micros();
  // Odczyt co 100 ms (100000 µs)
  if ((current_tick - previous_tick) >= 5000) {
    // Pobierz dane z akcelerometru
    AccGyr->Get_X_Axes(accelData);

    // Pobierz dane z żyroskopu
    AccGyr->Get_G_Axes(gyroData);

    // Wyświetl dane na monitorze szeregowym
    sprintf(report,
            "AccX: %d | AccY: %d | AccZ: %d | "
            "GyroX: %d | GyroY: %d | GyroZ: %d\n",
            accelData[0], accelData[1], accelData[2],
            gyroData[0], gyroData[1], gyroData[2]);
    Serial.print(report);

    // Zresetuj znacznik czasu
    previous_tick = micros();
  }
}
