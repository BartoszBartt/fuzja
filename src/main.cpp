#include <Arduino.h>
#include <LSM6DSLSensor.h>
#include <Wire.h>
#include <math.h>

LSM6DSLSensor *AccGyr;

float gyroNoise[3] = {0.015f * 0.015f, 0.015f * 0.015f, 0.005f * 0.005f}; // Szum żyroskopu
float accNoise[3] = {0.015f * 0.015f, 0.015f * 0.015f, 0.005f * 0.005f};  // Szum akcelerometru
float P[4][4] = { // Macierz kowariancji
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}
};
float q[4] = {1.0, 0.0, 0.0, 0.0}; // Początkowy kwaternion

const float sampling_dt = 0.01; // Stały czas próbkowania w sekundach (100 Hz)

// Funkcje pomocnicze
void normalizeQuaternion(float *q) {
  float norm = sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
  for (int i = 0; i < 4; i++) q[i] /= norm;
}

// Funkcja predykcji
void predict(float *q, float *gyro, float dt) {
  float qw = q[0], qx = q[1], qy = q[2], qz = q[3];
  float wx = gyro[0], wy = gyro[1], wz = gyro[2];

  float dq[4] = {
    -qx * wx - qy * wy - qz * wz,
     qw * wx + qy * wz - qz * wy,
     qw * wy - qx * wz + qz * wx,
     qw * wz + qx * wy - qy * wx
  };

  for (int i = 0; i < 4; i++) {
    q[i] += dt * dq[i] * 0.5f;
  }

  normalizeQuaternion(q);
}

// Funkcja aktualizacji
void update(float *q, float *acc, float P[4][4], float *gyroNoise, float *accNoise) {
  // Normalizacja danych akcelerometru
  float accNorm = sqrt(acc[0] * acc[0] + acc[1] * acc[1] + acc[2] * acc[2]);
  for (int i = 0; i < 3; i++) acc[i] /= accNorm;

  // Przewidywany wektor grawitacji
  float g[3] = {
    2.0f * (q[1] * q[3] - q[0] * q[2]),
    2.0f * (q[0] * q[1] + q[2] * q[3]),
    q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3]
  };

  // Różnica pomiędzy pomiarem a przewidywaniem
  float v[3] = {acc[0] - g[0], acc[1] - g[1], acc[2] - g[2]};

  // Uproszczona aktualizacja kwaternionu
  for (int i = 1; i < 4; i++) {
    q[i] += v[i - 1];
  }

  normalizeQuaternion(q);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Inicjalizacja czujnika
  AccGyr = new LSM6DSLSensor(&Wire);
  AccGyr->begin();
  AccGyr->Enable_X();
  AccGyr->Enable_G();
}

void loop() {
  int32_t rawAcc[3], rawGyro[3];
  float acc[3], gyro[3];

  AccGyr->Get_X_Axes(rawAcc);
  AccGyr->Get_G_Axes(rawGyro);

  // Konwersja danych do float
  for (int i = 0; i < 3; i++) {
    acc[i] = rawAcc[i] * 0.001f;  // Przykładowa konwersja (dostosuj w razie potrzeby)
    gyro[i] = rawGyro[i] * 0.0174533f; // Deg/s -> Rad/s
  }

  // EKF: Predykcja i aktualizacja
  predict(q, gyro, sampling_dt);
  update(q, acc, P, gyroNoise, accNoise);

  // Wylicz kąty Eulera
  float roll = atan2(2.0f * (q[0] * q[1] + q[2] * q[3]), 1.0f - 2.0f * (q[1] * q[1] + q[2] * q[2]));
  float pitch = asin(2.0f * (q[0] * q[2] - q[3] * q[1]));
  float yaw = atan2(2.0f * (q[0] * q[3] + q[1] * q[2]), 1.0f - 2.0f * (q[2] * q[2] + q[3] * q[3]));

  // Wyświetlanie wyników
  Serial.print("Roll: "); Serial.print(roll * 57.2958); // Radiany -> Stopnie
  Serial.print(" Pitch: "); Serial.print(pitch * 57.2958);
  Serial.print(" Yaw: "); Serial.println(yaw * 57.2958);

  delay(10); // Czas próbkowania 100 Hz
}
