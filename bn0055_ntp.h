#include <NTPClient.h>  // https://github.com/arduino-libraries/NTPClient
//#include <WiFiUdp.h>    // https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/udp-examples.html

//WiFiUDP UDP;                                           //
//NTPClient NTPClient(UDP, wifi.MQTTServer, 0, 600000);  // local time offset is -14400 (if used); autoresync every 600,000 msec (10 min)

struct ntp {
  bool sync=false;
  unsigned long lastupdate = 0;  // timestamp (in local millis) of last NTP sync.
} ntp;
