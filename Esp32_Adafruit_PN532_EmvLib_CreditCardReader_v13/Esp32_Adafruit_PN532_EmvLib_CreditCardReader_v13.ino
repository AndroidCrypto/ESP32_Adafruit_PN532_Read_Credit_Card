/*
  This is the accompanying sketch to the article:
  Read an EMV (Credit-) card with your ESP32 and PN532 NFC Reader.
  
  It is reading a Credit Card and exposes the Credit Card Number ("PAN") and the
  expiration date.
  
  The communication with the card is done by an ESP32 connected to a
  PN532 NFC card reader that is driven by the Adafruit_PN532 library.

  Created by AndroidCrypto (Michael Fehr 2025)
*/

// --------------------------------------------------------------
// Programm Information
const char *PROGRAM_VERSION = "ESP32 Adafruit_PN532 EMV Library Credit Card Reader V13";

#define RUN_EMV01_CREDIT_CARD

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

/*
// settings for ESP32-S3
#define PN532_SCK (42)
#define PN532_MOSI (2)
#define PN532_SS (40)
#define PN532_MISO (41)
const uint8_t tftWidth = 240;
const uint32_t tftHeight = 320;
*/

// settings for ESP32 ST7789 1.9-inches TFT display
#define PN532_SCK (33)
#define PN532_MOSI (32)
#define PN532_SS (25)
#define PN532_MISO (34)
const uint8_t tftWidth = 170;
const uint32_t tftHeight = 320;

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ (-1)
#define PN532_RESET (-1)  // Not connected by default on the NFC Shield

// Uncomment just _one_ line below depending on how your breakout or shield
// is connected to the Arduino:

// Use this line for a breakout with a SPI connection:
//Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

// Use this line for a breakout with a hardware SPI connection.  Note that
// the PN532 SCK, MOSI, and MISO pins need to be connected to the Arduino's
// hardware SPI SCK, MOSI, and MISO pins.  On an Arduino Uno these are
// SCK = 13, MOSI = 11, MISO = 12.  The SS line can be any digital IO pin.
//Adafruit_PN532 nfc(PN532_SS);

// Or use this line for a breakout or shield with an I2C connection:
//Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

#include "ESP32_EMV.h"

ESP32_EMV emv(&nfc);

void printHex(byte *buffer, uint16_t bufferSize);

const char *DIVIDER = "-------------------------------------------------------------------------";

boolean success;
char scrBuf[60];                          // buffer for tft outputs
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

#include "E01_CreditCardReader.h"

void setup(void) {
  Serial.begin(115200);
  //while (!Serial) delay(10);  // for Leonardo/Micro/Zero
  delay(500);
  Serial.println(PROGRAM_VERSION);

  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1)
      ;  // halt
  }

  // Got ok data, print it out!
  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);

  Serial.printf("ESP32_EMV library version: %d\n", emv.EMV_LIBRARY_VERSION);

  Serial.println("Waiting for an ISO14443A card");
}

void loop(void) {

  success = nfc.inListPassiveTarget();

  if (success) {
    Serial.println("Found a card!");

    bool DO_STOP = true;
   

#ifdef RUN_EMV01_CREDIT_CARD
  run_E01_Credit_Card_Handling();
  delay(1000);

#endif

  Serial.println(DIVIDER);
  delay(1000);
  }
}

void printHex(byte *buffer, uint16_t bufferSize) {
  for (uint16_t i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printHexShort(byte *buffer, uint16_t bufferSize) {
  for (uint16_t i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? "0" : "");
    Serial.print(buffer[i], HEX);
  }
}