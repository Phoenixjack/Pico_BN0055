TARGET HARDWARE: RASPBERRY PI PICO W + BOSCH BN0055 + uBLOX NEO-7M
AUTHOR: CHRIS MCCREARY
VERSION: 0.0.1
DATE: 2024-11-21

KEY NOTES:
For saving to SD card, filenames are limited to 8+3 in length.
To keep session names unique, a session is created with a random 16 bit / 2 byte / 4 hex character ID (0x0000-0xFFFF).
Session IDs will serve as the prefix for each session.
Each session will hold an index file (XXXXindx.txt) and timestamp file (XXXXtime.txt).
  The index file will hold any header info.
  The timestamp file will only be created once we have a valid epoch timestamp.
Each data file will start with the session ID, and be suffixed with a 16 bit sequence number, for a maximum of 65,535 data files per session.
Each data file has an arbitrary limit of 10Kb before it starts a new data file.
Assuming an average data entry size of ~160bytes, this should give us:
  ~6400 entries per data file
  ~419,424,000 entries per session
Assuming a data entry once per second, this should give us:
  ~6,990,400 minutes / 116,506.6 hours of runtime
Assuming an 8Gb SD card, this should give us:
  ~13 maximum sessions before the disk is full
THEORETICALLY! Theoretically, I should be dating Jessica Alba.

## V0_0_1:
[X] GPS integration
[ ] Should we report pitch/roll?
[ ] We're only using a single core; can it be split between 2?
[ ] TODO: Add packet parsing on the fly, retain count/GID/UID/RSSI/NOISE

## V0_0_0:
[X] first running version
[X] uses PhoenixJack_serialbuffer_V0 library for non-blocking UART collection
[X] queries BN0055 and all UART ports in main loop
[X] gathers health packet data on UART0 (Serial1) at 115.2K and holds it in buffer array
[X] once packet complete, assembles JSON packet, and includes the latest compass reading
