#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

Adafruit_BNO055 bno = Adafruit_BNO055(276, 0x28);

struct bn0055 {
  bool ready = false;
  uint8_t min_read_interval_ms = 100;
  uint32_t next_read = 0;
  uint32_t last_read = 0;
  float x, y, z;
  uint8_t sys, gyro, accel, mag = 0;
  bool init();
  bool check(bool get_cal_data);
  void displaySensorDetails();
} bn0055;

bool bn0055::init() {
  if (bno.begin()) {
    ready = true;
    bno.setExtCrystalUse(true);
    bno.getCalibration(&sys, &gyro, &accel, &mag);
    displaySensorDetails();
  } else {
    ready = false;
  }
  return ready;
};

bool bn0055::check(bool get_cal_data = false) {
  if (millis() > next_read && ready) {
    sensors_event_t event;
    bno.getEvent(&event);
    x = (float)event.orientation.x;
    y = (float)event.orientation.y;
    z = (float)event.orientation.z;
    if (get_cal_data) {
      bno.getCalibration(&sys, &gyro, &accel, &mag);
    }
    last_read = millis();
    next_read = last_read + (uint32_t)min_read_interval_ms;
    return true;
  } else {
    return false;
  }
};

void bn0055::displaySensorDetails() {
  sensor_t sensor;
  bno.getSensor(&sensor);
  Serial.println("\n\n\n------------------------------------");
  Serial.print("Sensor:       ");
  Serial.println(sensor.name);
  Serial.print("Driver Ver:   ");
  Serial.println(sensor.version);
  Serial.print("Unique ID:    ");
  Serial.println(sensor.sensor_id);
  Serial.print("Max Value:    ");
  Serial.print(sensor.max_value);
  Serial.println(" xxx");
  Serial.print("Min Value:    ");
  Serial.print(sensor.min_value);
  Serial.println(" xxx");
  Serial.print("Resolution:   ");
  Serial.print(sensor.resolution);
  Serial.println(" xxx");
  Serial.println("------------------------------------");
  Serial.println("");
}
