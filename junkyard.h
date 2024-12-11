
/*

void establish_interval(uint32_t latest_msg_timestamp) {
    if (last_msg_rcvd > 0) {
      uint32_t curr_interval_msec = latest_msg_timestamp - last_msg_rcvd;
      if (last_interval_msec > 0) {
        //uint32_t difference = (last_interval_msec > curr_interval_msec) ? (last_interval_msec - curr_interval_msec) : (curr_interval_msec - last_interval_msec);
        uint32_t difference = abs(last_interval_msec - curr_interval_msec);
        float deviation = difference / min(last_interval_msec, curr_interval_msec);
        if (deviation < 20) {
          avg_interval_msec = avg_interval_msec * received_msgs;
          avg_interval_msec += difference;
          avg_interval_msec /= (received_msgs + 1);
        }
      }
      last_interval_msec = curr_interval_msec;
    }
  };



  
deserializeJson(doc, Serial); // Parse input DIRECTLY from the serial port

// Computes the size of the document that serializeMsgPack() produces
https://arduinojson.org/v7/api/msgpack/measuremsgpack/

// Allows inserting preformatted pieces of JSON or MsgPack in a document
https://arduinojson.org/v7/api/misc/serialized/ 



JsonDocument filter;
filter["list"][0]["temperature"] = true;
deserializeJson(doc, input, DeserializationOption::Filter(filter));


const char* city = doc["city"];
float temp = doc["temperature"];
auto city = doc["city"].as<const char*>();
auto temp = doc["temperature"].as<float>();
https://en.cppreference.com/w/c/chrono/tm
struct tm timeinfo = *gmtime(&time);
doc["time"] = timeinfo; // set {"time":"2021-05-04T13:13:04Z"}
timeinfo = doc["time"]; // parse {"time":"2021-05-04T13:13:04Z"}

void send_msg() {
  JsonDocument doc;
  doc["sensor"] = "gps";
  doc["time"] = 1351824120;
  String jsonstring;
  serializeJson(doc, jsonstring);
  // serializeJsonPretty(doc, Serial);

  bool retained = false;
  int qos = 1;
  bool dup = false;
  mqttClient.beginMessage(outTopic, payload.length(), retained, qos, dup);
  mqttClient.print(payload);
  mqttClient.endMessage();

}

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