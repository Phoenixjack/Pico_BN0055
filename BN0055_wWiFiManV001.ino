#define def_column_labels "msec_uptime,epoch_sec,mag_heading,lat,lon,packet_num,gid,uid,tempC,vcc,dut_rssi,dut_noise,stick_rssi,stick_noise,gps"

#include <PhoenixJack_serialbuffer_V0.h>
PhoenixJack_serialbuffer SerMon(&Serial);   // Serial Monitor
PhoenixJack_serialbuffer MiniDL(&Serial1);  // Pin 2 = RX0 // 115200
#include "bn0055_gpio.h"
#include "bn0055_report.h"
#include "bn0055_sensor.h"
#include "bn0055_gps.h"
#include "PhoenixJack_serializeddatalogger_V2.h"
//#include "PhoenixJack_lora_v0.h"
//#include "bn0055_wifi.h" // the end goal is https://gitlab.com/painlessMesh/painlessMesh
//#include "bn0055_mqtt.h"
//#include "bn0055_ntp.h"

struct subsystems {  // currently in bypass while troubleshooting
  bool gps = false;
  bool bn55 = false;
  bool sdcard = false;
  bool lcd = false;
  bool wifi = false;
  bool mqtt = false;
  bool lora = false;
  bool minidl = false;  // goes true if we've received packets
  bool minimum_required_to_boot() {
    //return (gps & bn55 & sdcard);  // include any boolean flags that we ABSOLUTELY must have to function
    return sdcard;
  };
} subsystems;

struct packet_stats {
  uint32_t received_msgs = 0;
  uint32_t last_msg_rcvd = 0;  // millis() timestamp
  bool interval_established = false;
  uint32_t curr_interval_msec = 0;
  uint32_t avg_interval_msec = 0;
  uint32_t expected_earliest = 0;
  uint32_t expected_latest = 0;
  uint16_t missed_packets_consecutive = 0;
  uint16_t missed_packets_total = 0;
  void establish_interval() {  // should we sanity check our curr_interval_msec before we go averaging it in?
    static uint8_t valid_msgs = 1;
    avg_interval_msec *= (valid_msgs);
    avg_interval_msec += curr_interval_msec;
    valid_msgs++;
    avg_interval_msec /= valid_msgs;
    if (valid_msgs > 10) { interval_established = true; }
  };
  void process_new_msg(uint32_t latest_msg_timestamp) {
    received_msgs++;
    if (last_msg_rcvd > 0) { curr_interval_msec = latest_msg_timestamp - last_msg_rcvd; }
    if (interval_established) {
      if (latest_msg_timestamp > expected_earliest && latest_msg_timestamp < expected_latest) {  // check against our established avg_interval_msec (expected_earliest & expected_latest)
        missed_packets_consecutive = 0;
      } else {
        missed_packets_consecutive++;
        missed_packets_total++;
      }
      expected_earliest = millis() + (avg_interval_msec * 0.9);  // update our next expected window
      expected_latest = millis() + (avg_interval_msec * 1.1);
    } else {
      if (curr_interval_msec > 0) { establish_interval(latest_msg_timestamp); }
    }
    last_msg_rcvd = latest_msg_timestamp;
  };
} packet_stats;

void setup() {
  SerMon.init(115200, "BN0055\n\n");
  MiniDL.init(115200);
  gpio.init();                      // start up the indicator LEDs and dip switches
  subsystems.bn55 = bn0055.init();  // initialize the sensor
  if (subsystems.bn55) {
    Serial.println("BN0055: GOOD");
  } else {
    Serial.println("BN0055: BAD");
  }
  subsystems.sdcard = sd.init();  // initialize the sd card
  if (subsystems.sdcard) {
    sd.start_new_session(def_column_labels);  // start an SD session ID (random 0x0000 to 0xFFFF), and preload our CSV labels that will go in every file
    Serial.println("SD CARD: GOOD");
  } else {
    Serial.println("SD CARD: BAD");
  }
  subsystems.gps = gps.init();  // gps.init listens for 1.5 seconds; if any valid GPS sentence is received during that time, it returns true
  if (subsystems.gps) {
    Serial.println("GPS: GOOD");
  } else {
    Serial.println("GPS: BAD");
  }
  // subsystems.lcd ...
  // subsystems.lora ...
  if (subsystems.minimum_required_to_boot()) {  // aggregate the required systems
    onboardled.toggle_blocking(5, 250, 750);    // 5 long flashes to let the user we're good
    Serial.println("MINIMUM BOOT REQUIREMENTS MET");
  } else {
    Serial.println("MINIMUM BOOT REQUIREMENTS NOT MET");
  }
  gpio.set_runtime_states();
  Serial.println("GPIO RUNSTATES SET");
}

void loop() {
  //SerMon.check();                               // check for input from the Serial Monitor; CURRENTLY DOESN'T DO ANYTHING WITH THE INPUT

  gpio.check();  // query any input buttons/switches and update LEDs as needed

  report.file_save = SW_FILESAVE.check();  // interrogates the switch setting every XXXmsec and applies that to a flag

  if (bn0055.check()) {                   // query the sensor
    report.curr_heading = bn0055.x;       // copy the heading to the report struct; good for deconflicting dependencies
    report.heading_timestamp = millis();  // mark it with a cross and put it in the oven
  }                                       // for baby and me

  gps.check_and_act();  // checks GPS and acts on it at the same time; REMEMBER: this loads data to the report struct

  MiniDL.check();                                           // query the MiniDL UART port
  if (MiniDL.is_Buffer_Ready_To_Send()) {                   // so we'll only attempt to write to file when we have new MiniDL Data Packet
    strcpy(report.packet_buffer, MiniDL.buffer);            // store it in the report buffer for later usage; it's an extra step, but it'll free the receive buffer
    MiniDL.reset();                                         // flush it just in case, and reopen the receive buffer for new data
    packet_stats.process_new_msg(MiniDL.get_start_time());  // keep track of stats
    if (packet_stats.missed_packets_consecutive > 5) {      // if we've missed more than 5 packets in a row, we have a problem
      LED_Fault.toggle_nonblocking();
    } else {
      LED_Fault.set_off();
    }
    subsystems.sdcard = sd.isReady();                 // check our current status
    if (subsystems.sdcard && report.file_save) {      // only if it's good do we attempt to write;
      if (sd.write(report.get_CSV_data())) {          // if it was successful,
        Serial.println(millis() + "saved to file!");  // for debugging; remove later
        LED_FWri.single_flash_nonblocking(400);       // turn the LED on for xxx msec IFF we were able to successfully write to file
      }
      if (!report.time_set && report.epoch_time > 0) {  // IFF the time hasn't been set before AND we have a GPS referenced epoch time
        if (sd.mark_the_time(report.epoch_time)) {      // attempt to write the time to file
          report.time_set = true;                       // if it worked, then set the flag so we don't try again
        }
      }
    }
  }

  if (report.can_send()) {  // if auto-report is enabled AND it's time to send; used to throttle wifi/lora packets
    report.send_packet();   // send it
  }
}
