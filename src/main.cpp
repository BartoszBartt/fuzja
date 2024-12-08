#include <Arduino.h>
#include <LSM6DSLSensor.h>
#include <LSM303AGR_ACC_Sensor.h>
#include <LSM303AGR_MAG_Sensor.h>
#include <LPS22HHSensor.h>
#include <HTS221Sensor.h>

#define numAxes 3
LSM6DSLSensor *AccGyr;
LSM303AGR_ACC_Sensor *Acc;
LSM303AGR_MAG_Sensor *Mag;
LPS22HHSensor *PressTemp;
uint32_t previous_tick = 0;
uint32_t current_tick = 0;
char report[256];
void setup() {
// put your setup code here, to run once:
Serial.begin(115200);
Wire.begin();
AccGyr = new LSM6DSLSensor(&Wire);
AccGyr->begin();
AccGyr->Enable_X();
previous_tick = micros();
}
void loop() {
// put your main code here, to run repeatedly:
int32_t sensorData[numAxes];
current_tick = micros();
if((current_tick - previous_tick) >= 100000){
AccGyr->Get_X_Axes(sensorData);
sprintf(report, "AccX: %d | AccY: %d | AccZ: %d \n",
sensorData[0],sensorData[1],sensorData[2]);
Serial.print(report);
previous_tick = micros();
}
}