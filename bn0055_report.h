#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include "PhoenixJack_transmission_functions.h"
#define send_as_json true
#define send_as_csv false

struct report {                            // a global struct for storing report data to be sent out
                                           // BN0055 SENSOR INFO
  float curr_heading = 0;                  // for storing last heading
  uint32_t heading_timestamp = 0;          // millis() timestamp of when the sensor was read
                                           // PACKET DATA INFO
  char packet_buffer[200];                 // for storing the health packet received
  uint32_t packet_start_time = 0;          // millis() timestamp of when the packet started
                                           // GPS INFO
  uint32_t gps_time_stamp = 0;             // millis() timestamp of when the gps was processed
  uint8_t gps_seconds_since_last_fix = 0;  // used to track GPS.secondsSinceFix()
  float lat, lon, altitude;                // gps data
  uint8_t fixquality = 0;                  //
  bool gps_lock = false;                   //
  uint32_t epoch_time = 0;                 // holds GPS time as epoch timestamp in seconds
                                           // OUTGOING MESSAGE INFO
  bool enabled = true;                     // flag for enabling automatic reporting
  bool time_set = false;                   // flag for tracking if we've saved the time for this SD session
  bool file_save = false;                  // should we save to a local SD card?
  bool transmit_format = send_as_json;     // false for CSV, true for JSON
  uint16_t interval_ms = 500;              // how often can we send a report
  uint32_t next_send = 0;                  // pre-computed millis() time for next send; used for throttling wifi or lora output
  uint32_t sent_msgs = 0;                  // outgoing message counter -> build this in later for diagnostics
  bool can_send();                         // check the enabled flag and the next send
  void send_packet();                      //
  String get_JSON_data();                  // assemble data into a JSON string
  String get_CSV_data();                   // assemble data into a CSV string
} report;

bool report::can_send() {
  if (enabled && millis() > next_send) {
    return true;
  } else {
    return false;
  }
}

String report::get_JSON_data() {
  JsonDocument doc;
  doc["msg_id"] = sent_msgs;           // for tracking discrepancies / missed messages
  doc["uptime"] = millis();            // for server-side detection of delayed data and reboots
  doc["heading"] = curr_heading;       // just the heading for now
  doc["data"] = packet_buffer;         // the latest health packet
  doc["data_ts"] = packet_start_time;  // for more precise time sync functions (references the millisecond uptime that the message started)
  if (epoch_time != 0) {               // only include the epoch time IFF we have a GPS time lock in the first place
    doc["epoch"] = epoch_time;         // store it
  }
  if (gps_lock) {  // only include GPS data if we have it to reduce message size
    doc["lat"] = lat;
    doc["lon"] = lon;
    doc["alt"] = altitude;
  }
  String jsonstring;
  serializeJson(doc, jsonstring);                    // streamline into one long character string
  uint16_t cs16 = calculateChecksum16(&jsonstring);  // calculate checksum as 16bit value
  jsonstring += String(cs16, HEX);                   // append checksum as hex
  return jsonstring;
};

String report::get_CSV_data() {                  // "msec_uptime,epoch_sec,mag_heading,lat,lon,packet_num,gid,uid,tempC,vcc,dut_rssi,dut_noise,stick_rssi,stick_noise,gps"
  String csv_string = String(epoch_time) + ",";  // msec_uptime is automatically prepended by the PhoenixJack_serializeddatalogger library
  csv_string += String(curr_heading) + ",";
  csv_string += String(lat) + ",";
  csv_string += String(lon) + ",";
  csv_string += packet_buffer;
  return csv_string;
};

void report::send_packet() {  // just print to Serial Monitor for now
  sent_msgs++;                // keep count
  Serial.print("#" + sent_msgs + " -> ");
  if (transmit_format) {
    Serial.println(get_JSON_data());
  } else {
    Serial.println(get_CSV_data());
  }
  next_send = millis() + interval_ms;  // update the tracker
}