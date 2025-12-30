/**
 * A library for ESP32 and PN532 NFC Reader modules.
 * There are just a minimum of commands implemented that are
 * good for a simple 'read and write' tutorial.
 * For authentication the modern 'AuthenticateEV2First' method is used.
 *
 * The library is largely based on the outstanding work of Piotr Obst in developing 
 * the MFRC522_NTAG424DNA library, so all credit goes to him.
 * https://github.com/Obsttube/MFRC522_NTAG424DNA. License: MIT. 
 *
 * The communication is based on the Adafruit_PN532 library:
 * https://github.com/adafruit/Adafruit-PN532 version 1.3.4
 * The library needs to get modified in one file: 'Adafruit_PN532.cpp'
 * In line 78 change one parameter
 * old: #define PN532_PACKBUFFSIZ 64 ///< Packet buffer size in bytes
 * new: #define PN532_PACKBUFFSIZ 255 ///< Packet buffer size in bytes
 *
 * Author: Michael Fehr (AndroidCrypto)
*/

/*
 * Known restrictions with this implementation
 * - all read and write data file operations are limited to 256 bytes, as the parameter is just a byte
 * - Don't use FULL/encrypted record files with record sizes > 32 bytes, as the reading requires a decryption
 *   that seem to write into not allocated memory areas. This can be a reason for crashes, so stay on 32 bytes please.
Change in Adafruit_PN532.cpp
#define PN532_PACKBUFFSIZ 64                ///< Packet buffer size in bytes
to #define PN532_PACKBUFFSIZ 253            ///< Packet buffer size in bytes
byte pn532_packetbuffer[PN532_PACKBUFFSIZ]; ///< Packet buffer used in various
                                            ///< transactions
*/

/**
 * Version Management 
 * Vxx xx.11.2025  
 *
 * V13 30.11.2025 Based on V11, renaming 'SendAfl' to 'ReadRecord'
 * V12 29.11.2025 Testversion to read a record from a Visa Debit Card that is 255 bytes long
 *                Using a modified Adafruit_PN532 library. 'SendAfl' should name 'ReadRecord'
 * V11 20.11.2025 Impl. SelectApdu_Le, SendPdol_Le and SendAfl_Le
 * V10 19.11.2025 Version is based on V08, Impl. simple LookUpAid
 * V09 18.11.2025 Testversion for Arduino_PN532 trying to read a 255 bytes long file
 * V08 17.11.2025 Library is using new Status Codes 'EMV_StatusCode'
 * V07 15.11.2025 Renaming from ESP32_DESFire library to ESP32_EMV library
 * V05 15.11.2025 Impl. SendAfl 
 * V04 15.11.2025 SendPdol working for Master + VisaCard, all DESFire methods removed
 * V03 14.11.2025 Impl. SendPdol not working
 * V02 14.11.2025 Improve SelectApdu to find 
 * V01 14.11.2025 Impl. SelectApdu, Impl. SelectPpse
*/

/*
  Workflow for reading an EMV (Credit-/Payment card)
  1 Select PPSE
  2 Select APDU
  3 Send PDOL
  4 Send AFL
*/


/*
Wiring of the RED PN532 NFC module to an ESP32 T-Display using SPI interface
Pins seen from chip side of the module from LEFT to RIGHT
PN532 - ESP32
SCK   - 15
MISO  - 12
MOSI  - 13 
SS/CS - 17
VCC   - 3.3V 
GND   - GND

all PINs are on the left side of the dev board

#define PN532_SCK (15)
#define PN532_MOSI (13)
#define PN532_SS (17)  // CS
#define PN532_MISO (12)
*/

#ifndef ESP32_EMV_h
#define ESP32_EMV_h

#include "Arduino.h"
#include "Adafruit_PN532.h"

// For reading EMV Cards - a BER-TLV encoder/decoder
#include "tlv.h" // https://github.com/jmwanderer/tlv.arduino Arduino Library Manager Version 0.2.1

class ESP32_EMV {

public:

  /////////////////////////////////////////////////////////////////////////////////////
  // Contructors
  /////////////////////////////////////////////////////////////////////////////////////

  ESP32_EMV(Adafruit_PN532* nfc);

  // Credit Card Data
  const uint8_t EMV_LIBRARY_VERSION = 13;
  bool COMM_DEBUG_PRINT = true; // if true the send and received data is printed
  bool METHOD_DEBUG_PRINT = true; // if true some results are printed from inside a method
  bool TLV_DEBUG_PRINT = true; // if false the response is analyzed but not printed
  bool PDOL_DEBUG_PRINT = true; // true the response of PDOL lookup is printed

  // // byte[] PPSE = "2PAY.SYS.DDF01".getBytes(StandardCharsets.UTF_8); // PPSE
  byte SELECT_PPSE_COMMAND[14] = { 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31 };

  // BER-TLV decoder
  uint8_t buffer[255];
  TLVS tlvs;
  TLVNode *tlvNode, *childNode;
  size_t data_size;

  // 10 aids of max 16 bytes length, filled by SelectApdu SearchIndex 1 = after selectPpse
  uint8_t numberOfAids = 0;
  uint8_t aids[10][16];
  uint8_t aidsLen[10];
  // tag 9f38 = PDOL, filled by SelectApdu SearchIndex 2 = after select AID
  uint8_t pdolLen = 255;
  uint8_t pdol[255]; // filled by SelectApdu SerarchIndex 2
  const uint8_t NUMBER_OF_RETRIES = 3;

  // tag 57 is 'old' Track 2 Equivalent Data including cc data
  uint8_t tag57Complete[255];
  uint8_t tag57CompleteLen = 255;

  // Tag57 is Track 2 Equivalent Data
  // Tag: 57 Length: 10
  //         43 96 00 33 08 75 86 20 D2 60 32 21 96 61 17 12
  byte t57Pan[20];
  byte t57PanLen = 0;
  byte t57ExpDate[10];
  byte t57ExpDateLen = 0; // 0 = not found

  // Tag5A / Tag5F24
  byte t5aPan[20];
  byte t5aPanLen = 0;
  byte t5f24ExpDate[10];
  byte t5f24ExpDateLen = 0; // 0 = not found

  char panChar[30];
  uint8_t panCharLen = 0;
  char expDateChar[10];
  uint8_t expDateCharLen = 0;

  // tag 94h = AFL = Application File Locator
  byte t94Afl[255];
  byte t94AflLen = 0;

  uint8_t afl[255];
  uint8_t aflLen = 255;

  //bool COMM_DEBUG_PRINT = true;             // if true the send and received data is printed
  //bool AUTHENTICATION_DEBUG_PRINT = false;  // if true the complete authentication workflow is printed

  enum EMV_StatusCode : byte {
    EMV_STATUS_OK = 0,            // SUCCESS
    EMV_STATUS_ERROR = 1,         // Not specified error
    EMV_STATUS_NO_RESPONSE = 2,   // EMV card returns 255 bytes
    //EMV_STATUS_LE_LENGTH_00 = 3   // EMV card returns no data but wants the command replied with an Le length of '0x00h'
  };

  // Limitations on PN532 readers
  const uint8_t MAX_BUFFER_SIZE = 255;  // the internal buffer is 128 - 3 for status bytes


  /////////////////////////////////////////////////////////////////////////////////////
  //
  // Credit Card Handling
  //
  /////////////////////////////////////////////////////////////////////////////////////

  EMV_StatusCode SelectPpse(byte* backReadData, uint16_t* backReadLen);
  EMV_StatusCode SelectApdu(byte* sendData, byte sendLen, byte searchIndex, byte* backReadData, uint16_t* backReadLen);
  EMV_StatusCode SelectApdu_Le(byte* sendData, byte sendLen, byte leByte, byte* backReadData, uint16_t* backReadLen);
  EMV_StatusCode SendPdol(byte* backReadData, uint16_t* backReadLen);
  EMV_StatusCode SendPdol_Le(byte* sendData, byte sendLen, byte leByte, byte* backReadData, uint16_t* backReadLen);
  bool CheckOneBytePdol(byte data);
  bool LookUpPdolOneByte(byte byte1, byte length, byte* resData, byte* resLength);
  bool LookUpPdolTwoByte(byte byte1, byte byte2, byte length, byte* resData, byte* resLength);
  EMV_StatusCode LookUpAid(byte* sendData, byte sendLen, uint8_t* aidNameIndex);

  EMV_StatusCode ReadRecord(byte* aflEntry, byte* appData, uint16_t* backReadLen);
  EMV_StatusCode ReadRecord_Le(byte* aflEntry, byte leByte, byte* appData, byte* backReadLen);

  // helper methods
  void printHex(byte* buffer, uint16_t bufferSize);
  void convertLargeInt2Uint8_t4Lsb(int& input, uint8_t* output);
  void convertInt2Uint8_t(int& input, uint8_t* output);
  int convertUint8_t3_2IntLsb(byte* input);
  int convertUint8_t4_2IntLsb(byte* input);
  void convertIntTo3BytesLsb(int input, byte* output);
  void revertAid(byte* aid, byte* revertAid);

  void hexCharacterStringToBytes(byte *byteArray, const char *hexString);
  byte nibble(char c);
  byte upperPartByte(byte data);
  byte lowerPartByte(byte data);
  void dumpByteArray(const byte * byteArray, const byte arraySize);
  //void EMV_StatusCodeDebugPrint(EMV_StatusCode statusCode);

private:

  Adafruit_PN532* emvLib;


protected:

  /////////////////////////////////////////////////////////////////////////////////////
  //
  // Protected functions
  //
  /////////////////////////////////////////////////////////////////////////////////////

  EMV_StatusCode EMV_BasicTransceive(byte* sendData, byte sendLen, byte* backData, byte* backLen);

  
};

#endif