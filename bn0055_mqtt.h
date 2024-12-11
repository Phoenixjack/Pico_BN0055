#include <string.h>
#include <ArduinoMqttClient.h>  // https://github.com/arduino-libraries/ArduinoMqttClient
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
// sprintf(buffer, "%06X", hexValue);
// const String willPayload = nodeid + ":OFFLINE";
// char* json = "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
// char* teststring = "{\"apply\":{\"mqttserver\":\"192.168.0.7\",\"report\":false,\"reportint\":750}}";  // {"apply":{"mqttserver":"192.168.0.7"}}
//  mqttClient.beginWill(willTopic, willPayload.length(), willRetain, willQos);
//  mqttClient.endWill();
MqttClient mqttClient(wifiClient);

static void onMqttMessageStatic(int messageSize) {
  /*
void parse_json(char* rcvdata) {
  DeserializationError error = deserializeJson(doc, rcvdata);
  if (!error) {
    char* sensor = doc["sensor"];
    long time = doc["time"];
  } else {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
  }
}
*/
}

struct mqtt {
  bool ready = false;
  const String nodeid = String(ESP.getChipId(), HEX);
  char broker[16] = "192.168.0.7";
  int port = 1883;
  char topic[10];
  bool init();
  void poll();
} mqtt;



bool mqtt::init() {
  String my_topic = nodeid + "/";
  strcpy(topic, my_topic.c_str());
  if (mqttClient.connect(broker, port)) {
    ready = true;
    Serial.print(F("CONNECTED! Subscribing to topic: "));
    Serial.println(topic);
    mqttClient.subscribe(topic);
    mqttClient.onMessage(onMqttMessageStatic);
  } else {
    ready = false;
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
  }
  return ready;
};

void mqtt::poll() {
  mqttClient.poll();
};
