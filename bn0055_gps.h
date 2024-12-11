// STAGES DATA IN THE REPORT STRUCT!!!
// REPORT STRUCT MUST BE CALLED FIRST!!!

#include <UnixTime.h>          // https://www.unixtimestamp.com/index.php
UnixTime stamp(0);             // a timestamp object with no time zone offset
#include <Adafruit_GPS.h>      // https://github.com/adafruit/Adafruit_GPS
#define GPSSerial Serial2      // tying Serial2 / TX1/RX1 / hardware pin 11 & 12 to GPS module
Adafruit_GPS GPS(&GPSSerial);  // initiate

struct gps {  //
  bool ready = false;
  bool init();
  bool check();
  void check_and_act();
  void stage_data();
} gps;

bool gps::init() {                                // initialize the port and try to configure it
  GPS.begin(9600);                                // usually 4800 or 9600 baud
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);   // may not work with all gps modules
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);  // ditto
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);      // 1 Hz update rate
  unsigned long timeout = millis() + 1500;        // set a listening timeout
  ready = false;                                  // just being paranoid; realistically, this is set in the struct
  while (millis() < timeout && !ready) {          // as long as we haven't timed out AND we haven't recieved a proper GPS sentence
    ready = check();                              // keep checking
  }
  return ready;
}

bool gps::check() {
  char c = GPS.read();
  report.gps_seconds_since_last_fix = (uint8_t)GPS.secondsSinceFix();
  return GPS.newNMEAreceived();
}

void gps::check_and_act() {  // checks and if ready, stages the data; simplifies our procedure calls
  if (check()) { stage_data(); }
}

void gps::stage_data() {             // store any GPS and time data in the report struct for later processing
  report.gps_time_stamp = millis();  // timestamp of when we processed this
  if (GPS.year == 0) {               // it's possible to get NMEA sentences WITHOUT even a timefix yet; if we don't even have that, don't set epoch time
    report.epoch_time = 0;           // this is Chris telling Chris that Chris has a problem
  } else {                           // I got 99 problems, but this ain't one
    stamp.setDateTime(GPS.year + 2000, GPS.month, GPS.day, GPS.hour, GPS.minute, GPS.seconds);
    report.epoch_time = stamp.getUnix();
  }
  if (GPS.parse(GPS.lastNMEA())) {  // only load GPS data if we have a GPS fix
    report.gps_lock = GPS.fix;
    report.fixquality = GPS.fixquality;
    if (report.gps_lock) {  // only load GPS data if we have a GPS lock
      report.lat = GPS.latitudeDegrees;
      report.lon = GPS.longitudeDegrees;
      report.altitude = GPS.altitude;
    } else {
      report.lat = 0;
      report.lon = 0;
    }
  }
}
