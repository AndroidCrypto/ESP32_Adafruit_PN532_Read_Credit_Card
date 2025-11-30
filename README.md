# ESP32 Adafruit PN532 Read a Credit Card
The tutorial explains how to read a Credit Card on an ESP32 with a connected PN5232 NFC reader.

This is the accompanying repository for my article "Read an EMV (Credit-) card with your ESP32 and PN532 NFC Reader" available here:

![Image 6](./images/esp32_s3_dev_pn532_desfire_01_500h.png)

## Important notes about the code
Please note: The sketch is just the tutorial code and does not contain any code for displaying any data on a TFT display ! 

The DESFire library contains just the code (commands) needed for the tutorial but not a full implementation of the DESFire EVx commands.

## Required Hardware (NFC Reader)

I'm recommending two NFC readers, but please be aware that I read some negative comments on the "Red" PCB one - some seem to have design flaws.

![Image 1](./images/pn532_blue_front_500w.png) ![Image 2](./images/pn532_blue_back_500w.png)

![Image 3](./images/pn532_red_front_500h.png) ![Image 4](./images/pn532_red_back_500h.png)

### SPI interface

To use the **SPI mode** you have to bring the switch in the right position:

![Image 5](./images/pn532_red_blue_switches_400h.png)

````plaintext
// settings for ESP32 ST7789 1.9-inches TFT display
#define PN532_SCK (33)
#define PN532_MOSI (32)
#define PN532_SS (25)
#define PN532_MISO (34)
#define PN532_IRQ (-1)   // not connected
#define PN532_RESET (-1) // not connected
// The VCC pin of the reader is connected to the 3.3V pin of the ESP32
````

````plaintext
// settings for ESP32-S3 Developmentboard with TFT Display
#define PN532_SCK (42)
#define PN532_MOSI (2)
#define PN532_SS (40)
#define PN532_MISO (41)
#define PN532_IRQ (-1)   // not connected
#define PN532_RESET (-1) // not connected
// The VCC pin of the reader is connected to the 3.3V pin of the ESP32
````

## Required Library
````plaintext
Adafruit_PN532 by Adafruit version 1.3.4 (https://github.com/adafruit/Adafruit-PN532)
````

## Documents

### NTAG424DNA documents
data sheet Rev. 3.0 — 31 January 2019: https://www.nxp.com/docs/en/data-sheet/NT4H2421Gx.pdf (NT4H2421Gx_NTAG_424_DNA_data_sheet_rev_3_0_2019_01_31.pdf)

Features and Hints Application note Rev. 2.0 — 4 March 2025: https://www.nxp.com/docs/en/application-note/AN12196.pdf (AN12196_NTAG424_Features_and_Hints_Rev_2_0_2025_03_04.pdf)

### Mifare DESFire Light documents

data sheet Rev. 3.3 — 5 April 2019: https://www.nxp.com/docs/en/data-sheet/MF2DLHX0.pdf (MF2DLHX0_MIFARE_DESFire_Light_data_sheet_rev_3_3_2019_04_05.pdf)

Features and Hints Application note Rev. 1.1 — 20 January 2020: https://www.nxp.com/docs/en/application-note/AN12343.pdf (AN12343_MIFARE_DESFire_Light_Features_and_Hints_Rev_1_1_2020_01_20.pdf)

### Mifare DESFire EV3 Short data sheet

Short data sheet Rev. 3.1 — 11 January 2024: https://www.nxp.com/docs/en/data-sheet/MF3D_H_X3_SDS.pdf (MF3D_H_X3_SDS_MIFARE_DESFire_EV3_short_data_sheet_Rev_3_1_2024_01_11.pdf)

### Additional documents

Mifare DESFire protocol: https://raw.githubusercontent.com/revk/DESFireAES/master/DESFire.pdf (NXP_MIFARE_DESFire_EV1_Protocol.pdf)

### AN12752_MIFARE_DESFire_EV3_feature_and_functionality_comparison

MIFARE DESFire EV3 feature and functionality comparison to other MIFARE DESFire products Rev. 1.4 — 24 July 2023: https://www.nxp.com/docs/en/application-note/AN12752.pdf (AN12752_MIFARE_DESFire_EV3_feature_and_functionality_comparison_Rev_1_4_2023_07_24.pdf)

## Development Environment
````plaintext
Arduino IDE Version 2.3.6 (Windows)
arduino-esp32 boards Version 3.2.0 (https://github.com/espressif/arduino-esp32)
````

## Implementations

![Image 7](./images/esp32_1_9_display_pn532_desfire_01_500h.png)

## Log file of the complete tutorial workflow
````plaintext
ESP32 Adafruit PN532 DESFire Starter V02
Found chip PN532
Firmware ver. 1.6
ESP32_DESFire library version: 2
Waiting for an ISO14443A card

Tag number: 1
Found a card!

-------------------------------------------------------------------------
 T01 Basic Handling
-------------------------------------------------------------------------
-------------------------------------------------------------------------
-------------------------------------------------------------------------
Get Free Memory
Send length 5
 90 6E 00 00 00
Recv length 5
 00 14 00 91 00
SUCCESS
GetFreeMemory response length 3 data: 00 14 00
-------------------------------------------------------------------------
Get Version
Send length 5
 90 60 00 00 00
Recv length 9
 04 01 01 33 00 18 05 91 AF
Send length 5
 90 AF 00 00 00
Recv length 9
 04 01 01 03 00 18 05 91 AF
Send length 5
 90 AF 00 00 00
Recv length 16
 04 35 68 DA 05 1A 90 20 82 62 30 30 34 23 91 00
SUCCESS
GetVersion response length 28 data: 04 01 01 33 00 18 05 04 01 01 03 00 18 05 04 35 68 DA 05 1A 90 20 82 62 30 30 34 23
-------------------------------------------------------------------------
Create an Application
Send length 11
 90 CA 00 00 05 56 78 9A 0F 85 00
Recv length 2
 91 00
SUCCESS
-------------------------------------------------------------------------
Create a Standard Data file with Free Access Rights
Send length 13
 90 CD 00 00 07 01 00 EE EE 20 00 00 00
Recv length 2
 91 9D
PERMISSION_DENIED ERROR
-------------------------------------------------------------------------
Select an Application
Send length 9
 90 5A 00 00 03 56 78 9A 00
Recv length 2
 91 00
SUCCESS
-------------------------------------------------------------------------
Create a Standard Data file with Free Access Rights
Send length 13
 90 CD 00 00 07 01 00 EE EE 20 00 00 00
Recv length 2
 91 00
SUCCESS
-------------------------------------------------------------------------
Get File Settings
Send length 7
 90 F5 00 00 01 01 00
Recv length 9
 00 00 EE EE 20 00 00 91 00
SUCCESS
-------------------------------------------------------------------------
File Settings for file 01
File Type         : 00 (Standard Data File)
File Options      : 00
File Comm Mode    : PLAIN
File RW/CAR AccRg : EE
File R/W    AccRg : EE
File Size         : 32
-------------------------------------------------------------------------
Write to a file with Free Access Rights
Generated data length 32 data: 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20
Send length 45
 90 8D 00 00 27 01 00 00 00 20 00 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 00
Recv length 2
 91 00
SUCCESS
-------------------------------------------------------------------------
Read from a file with Free Access Rights
Send length 13
 90 BD 00 00 07 01 00 00 00 20 00 00 00
Recv length 34
 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 91 00
SUCCESS
Read Data response length 32 data: 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20
-------------------------------------------------------------------------
 T01 Basic Handling END
-------------------------------------------------------------------------

````


![Image 8](./images/esp32_s3_dev_pn532_desfire_02_500w.png)
