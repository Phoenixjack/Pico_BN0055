#include <DNSServer.h>    // https://github.com/esp8266/Arduino/blob/master/libraries/DNSServer/examples/DNSServer/DNSServer.ino
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>  // https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html
WiFiClient wifiClient;


//if (wifi.init()) {
//  gpio.toggle_blocking(3, 750, 1000);
//}
struct wifi {
  char MQTTServer[40];
  char MQTTPort[6] = "1883";
  bool ready = false;
  bool init();
} wifi;


bool wifi::init() {
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", MQTTServer, 40);  // has to be declared globally; FOLLOWUP: IF we want to display the previously saved data, we'll need to load the file and parse before this point
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", MQTTPort, 6);           // has to be declared globally
  WiFiManager wm;
  ready = wm.autoConnect();  // auto generated AP name from chipid
  if (!ready) {
    Serial.println("Failed to connect");
  } else {
    Serial.println("connected...yeey :)");
  }
  return ready;
}
