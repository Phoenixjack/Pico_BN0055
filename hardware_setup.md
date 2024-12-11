
|GPIO|PIN#| FUNCTION  | - |     FUNCTION |PIN#|GPIO|
| -- | -  | --------  | - |     -------- | -- | -- |
|  0 |  1 | TX0       |   |         VBUS | 40 | -- |
|  1 |  2 | RX0       |   |         VSYS | 39 | -- |
| -- |  3 | GND       |   |          GND | 38 | -- |
|  2 |  4 |           |   |       3V3_EN | 37 | -- |
|  3 |  5 |           |   |          3V3 | 36 | -- |
|  4 |  6 | SDA0      |   |     ADC_VREF | 35 | -- |
|  5 |  7 | SCL0      |   |  SW_FILESAVE | 34 | 28 |
| -- |  8 | GND       |   |          GND | 33 | -- |
|  6 |  9 |           |   |              | 32 | 27 |
|  7 | 10 |           |   |              | 31 | 26 |
|  8 | 11 | TX1       |   |        RESET | 30 | -- |
|  9 | 12 | RX1       |   |              | 29 | 22 |
| -- | 13 | GND       |   |          GND | 28 | -- |
| 10 | 14 |           |   |              | 27 | 21 |
| 11 | 15 |           |   |              | 26 | 20 |
| 12 | 16 |           |   |    SPI0:MOSI | 25 | 19 |
| 13 | 17 | LED_FWri  |   |     SPI0:SCK | 24 | 18 |
| -- | 18 | GND       |   |          GND | 23 | -- |
| 14 | 19 | LED_Fault |   |      SPI0:CS | 22 | 17 |
| 15 | 20 |           |   |    SPI0:MISO | 21 | 16 |

RX0 = Serial1 (MiniDL)
RX1/TX1 = Serial2 (GPS)
LED_FWri = file write indicator; active high
LED_Fault = missed_packets_consecutive > 5; active high
SW_FILESAVE = file saving enable; active low (LED directly tied to switch)

TODO:
Fault indicator?
DIP_FORMAT = JSON or CSV format
LED_GPS = GPS lock indicator