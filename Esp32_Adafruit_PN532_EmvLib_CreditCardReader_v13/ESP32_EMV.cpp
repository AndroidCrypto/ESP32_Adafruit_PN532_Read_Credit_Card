#include "Arduino.h"
#include "ESP32_EMV.h"
#include <Adafruit_PN532.h>


/////////////////////////////////////////////////////////////////////////////////////
//
// Basic functions for communicating with EMV cards
//
/////////////////////////////////////////////////////////////////////////////////////

ESP32_EMV::ESP32_EMV(Adafruit_PN532* nfc) {
  emvLib = nfc;
}

/////////////////////////////////////////////////////////////////////////////////////
//
// Credit Card Handling
//
/////////////////////////////////////////////////////////////////////////////////////

ESP32_EMV::EMV_StatusCode ESP32_EMV::SelectPpse(byte* backReadData, uint16_t* backReadLen) {
  //uint16_t selectPpseLen = 14;
  // byte[] PPSE = "2PAY.SYS.DDF01".getBytes(StandardCharsets.UTF_8); // PPSE
  //byte selectPpse[14] = { 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31 };
  uint16_t backLen = 255;
  byte backData[backLen];

  EMV_StatusCode statusCode;
  statusCode = SelectApdu(SELECT_PPSE_COMMAND, sizeof(SELECT_PPSE_COMMAND), 0x01, backData, &backLen);

  if (statusCode != EMV_STATUS_OK) {
    *backReadLen = backLen;
    return (EMV_StatusCode)statusCode;
  }

  memcpy(backReadData, backData, backLen);
  *backReadLen = backLen;
  return EMV_STATUS_OK;
}

// SerarchIndex: 0 = no search, 1 = search for tag 4Fh = AID, 2 = search for tag 9F38h = PDOL
ESP32_EMV::EMV_StatusCode ESP32_EMV::SelectApdu(byte* sendData, byte sendLen, byte searchIndex, byte* backReadData, uint16_t* backReadLen) {

  if (METHOD_DEBUG_PRINT) {
    Serial.printf("SelectApdu searchIndex %02x\n", searchIndex);
  }

  //byte backData[256];
  byte backData[255];
  uint16_t backLen = 255;
  byte leByte = 0xF8;  // don't ask for the full length in the first run

  EMV_StatusCode statusCode;
  statusCode = SelectApdu_Le(sendData, sendLen, leByte, backData, &backLen);

  if (statusCode != EMV_STATUS_OK) return statusCode;

  if (backLen == 2) {
    if (METHOD_DEBUG_PRINT) Serial.printf("statusCode %d backLen %d\n", statusCode, backLen);
    if (backData[backLen - 2] == 0x67 && backData[backLen - 1] == 0x00) {
      if (METHOD_DEBUG_PRINT) {
        // this means the card is asking for a 'zero' Le
        Serial.println("------------------------");
        Serial.println("Card is asking for Le = 0x00");
      }

      leByte = 0x00;
      backLen = 255;
      statusCode = SelectApdu_Le(sendData, sendLen, leByte, backData, &backLen);
    }
  }

  if (backLen == 255) {
    uint8_t retries = 0;
    bool tryNewSend = true;
    while (tryNewSend) {
      if (METHOD_DEBUG_PRINT) {
        Serial.println("------------------------");
        Serial.printf("Retry No %d\n", retries + 1);
      }
      backLen = 255;
      statusCode = SelectApdu_Le(sendData, sendLen, leByte, backData, &backLen);
      if (backLen < 255) tryNewSend = false;
      retries++;
      if (retries == NUMBER_OF_RETRIES) tryNewSend = false;
    }
  }

  // if something gone wrong the backLen seems to be 255
  if (backLen < 255) {

    if (statusCode != EMV_STATUS_OK)
      return (EMV_StatusCode)statusCode;

    // BER-TLV decoder
    uint8_t buffer[255];
    //TLVS tlvs;
    TLVNode *tlvNode, *childNode;
    size_t data_size;
    memcpy(buffer, backData, backLen);
    tlvs.decodeTLVs(buffer, sizeof(buffer));

    int tlvsErrorValue = tlvs.errorValue();
    //Serial.printf("tlvsErrorValue %d\n", tlvsErrorValue);
    /*
    int tlvNodeErrorCodes = tlvNode->errorCodes();
    Serial.printf("tlvNodeErrorCodes %d\n", tlvNodeErrorCodes);
    int tlvChildNodeErrorCodes = childNode->errorCodes();
    Serial.printf("tlvChildNodeErrorCodes %d\n", tlvChildNodeErrorCodes);
    */

    // Dump the decoded TLV structure
    tlvNode = tlvs.firstTLV();

    if (METHOD_DEBUG_PRINT) {
      Serial.print("TLV Node ");
      Serial.println(tlvNode->getTag(), HEX);
    }
    for (childNode = tlvNode->firstChild(); childNode; childNode = tlvNode->nextChild(childNode)) {
      if (METHOD_DEBUG_PRINT) {
        Serial.print("Child Node ");
        Serial.println(childNode->getTag(), HEX);
      }
    }
    TLVS::printTLV(tlvNode);

    if (searchIndex == 0x01) {
      if (METHOD_DEBUG_PRINT) Serial.printf("Search for tag 4F (AIDs on card)\n");
      numberOfAids = 0;
      // find a tag
      TLVNode* tlvNodeSearch;
      uint16_t tag4F = 0x4F;
      tlvNodeSearch = tlvs.findTLV(tag4F);

      // don't proceed if result is NULL

      if (tlvNodeSearch != NULL) {

        const uint8_t* tag4FValue = tlvNodeSearch->getValue();
        uint32_t tag4FValueLength = tlvNodeSearch->getValueLength();

        if (METHOD_DEBUG_PRINT) Serial.printf("tlvNode->getValue length %d\n", tag4FValueLength);
        //printHex((byte) tag4FValue, sizeof(tag4FValue));
        //for (uint8_t i = 0; i < sizeof(tag4FValue); i++) {
        for (uint8_t i = 0; i < tag4FValueLength; i++) {
          if (METHOD_DEBUG_PRINT) Serial.printf("%02x ", tag4FValue[i]);
          aids[numberOfAids][i] = tag4FValue[i];
        }
        aidsLen[numberOfAids] = tag4FValueLength;
        numberOfAids++;

        while (tlvNodeSearch = tlvs.findNextTLV(tlvNodeSearch)) {
          if (tlvNodeSearch == NULL) {
            if (METHOD_DEBUG_PRINT) Serial.println("tlvNodeSearch = tlvs.findNextTLV() == NULL");
          } else {
            if (METHOD_DEBUG_PRINT) Serial.println("tlvNodeSearch = tlvs.findNextTLV() NOT NULL");
            const uint8_t* tag4FValueN;
            tag4FValueN = tlvNodeSearch->getValue();
            tag4FValueLength = tlvNodeSearch->getValueLength();
            if (METHOD_DEBUG_PRINT) Serial.printf("tlvNode->getValue length %d\n", tag4FValueLength);
            for (uint8_t i = 0; i < tag4FValueLength; i++) {
              if (METHOD_DEBUG_PRINT) Serial.printf("%02x ", tag4FValueN[i]);
              aids[numberOfAids][i] = tag4FValueN[i];
            }
            aidsLen[numberOfAids] = tag4FValueLength;
            numberOfAids++;
            if (METHOD_DEBUG_PRINT) Serial.println("***");
          }
        }
        if (METHOD_DEBUG_PRINT) Serial.printf("Found %d AIDs on the card\n", numberOfAids);
      }  // if tlvNodeSearch != NULL
    } else if (searchIndex == 0x02) {
      // search for tag 9F38 = PDOL
      if (METHOD_DEBUG_PRINT) Serial.printf("Search for tag 9F38 (PDOLs)\n");
      TLVNode* tlvNodeSearch;
      uint16_t tag9F38 = 0x9F38;
      tlvNodeSearch = tlvs.findTLV(tag9F38);

      // don't proceed if result is NULL
      // Tag: 9F38 Length: 6
      //      9F 02 06 9F 1D 02

      // Tag: 9F38 Length: 9
      //      9F 35 01 9F 40 01 9F 1D 02

      // Tag: 9F38 Length: 18
      //      9F 66 04 9F 02 06 9F 03 06 9F 1A 02 95 05 5F 2A 02 9A 03 9C 01 9F 37 04


      if (tlvNodeSearch != NULL) {

        const uint8_t* tag9F3AValue = tlvNodeSearch->getValue();
        uint32_t tag9F3AValueLength = tlvNodeSearch->getValueLength();

        if (METHOD_DEBUG_PRINT) Serial.printf("tlvNode->getValue length %d\n", tag9F3AValueLength);
        //printHex((byte) tag4FValue, sizeof(tag4FValue));
        //for (uint8_t i = 0; i < sizeof(tag4FValue); i++) {
        for (uint8_t i = 0; i < tag9F3AValueLength; i++) {
          if (METHOD_DEBUG_PRINT) Serial.printf("%02x ", tag9F3AValue[i]);
          pdol[i] = tag9F3AValue[i];
        }
        pdolLen = tag9F3AValueLength;
        if (METHOD_DEBUG_PRINT) Serial.println("*PDOL*");
      } else {
        pdolLen = 255;
      }
    }

    if (backLen > 2)
      memcpy(backReadData, backData, backLen - 2);
    *backReadLen = backLen - 2;

    return EMV_STATUS_OK;
  }
  *backReadLen = backLen;
  return EMV_STATUS_ERROR;
}

// This is the native code for SelectApdu. To be flexible this method allows to use alternative Le values (usually 0x00h)
ESP32_EMV::EMV_StatusCode ESP32_EMV::SelectApdu_Le(byte* sendData, byte sendLen, byte leByte, byte* backReadData, uint16_t* backReadLen) {
  if (METHOD_DEBUG_PRINT) {
    Serial.printf("SelectApdu leByte %02x sendLen %d data:\n", leByte, sendLen);
    printHex(sendData, sendLen);
    Serial.println();
  }

  byte* sendData2 = new byte[sendLen + 6];

  sendData2[0] = 0x00;           // CLA
  sendData2[1] = 0xA4;           // INS
  sendData2[2] = 0x04;           // P1
  sendData2[3] = 0x00;           // P2
  sendData2[4] = (byte)sendLen;  // Lc
  memcpy(&sendData2[5], sendData, sendLen);
  sendData2[sendLen + 5] = leByte;  // Le (0x00h)

  byte backData[255];
  byte backLen = 255;

  EMV_StatusCode statusCode;

  statusCode = EMV_BasicTransceive(sendData2, sendLen + 6, backData, &backLen);
  memcpy(backReadData, backData, backLen);
  *backReadLen = backLen;
  return statusCode;
}

ESP32_EMV::EMV_StatusCode ESP32_EMV::SendPdol(byte* backReadData, uint16_t* backReadLen) {
  // the data is in pdol and pdolLen
  EMV_StatusCode statusCode;
  byte backData[255];
  uint16_t backLen = 255;
  byte leByte;
  if (pdolLen > 254) {
    if (METHOD_DEBUG_PRINT) Serial.println("SendPdol is empty");
    // this is the MasterCard way, no PDOL is present and a zeroed PDOL is send
    // 80 A8 00 00 02 83 00 00
    backLen = 255;
    byte pdolEmpty[2];
    pdolEmpty[0] = 0x83;
    pdolEmpty[1] = 0x00;
    leByte = 0xF8;
    //leByte = 0xDF;
    statusCode = SendPdol_Le(pdolEmpty, sizeof(pdolEmpty), leByte, backData, &backLen);

    if (backLen == 2) {
      if (METHOD_DEBUG_PRINT) Serial.printf("statusCode %d backLen %d\n", statusCode, backLen);
      if (backData[backLen - 2] == 0x67 && backData[backLen - 1] == 0x00) {
        // this means the card is asking for a 'zero' Le
        if (METHOD_DEBUG_PRINT) Serial.println("Card is asking for Le = 0x00");
        backLen = 255;
        leByte = 0x00;
        //hexCharacterStringToBytes(pdolEmpty, pdolEmptyStringLe00);
        statusCode = SendPdol_Le(pdolEmpty, sizeof(pdolEmpty), leByte, backData, &backLen);
      }
    }

    /*
SendPdol is empty
Send length 8
 80 A8 00 00 02 83 00 00
Recv length 22
 77 12 82 02 19 80 94 0C 08 01 01 00 10 01 01 01 20 01 02 00 90 00
*/
  } else {
    if (METHOD_DEBUG_PRINT) Serial.printf("SendPdol is requested with length %d\n", pdolLen);

    // get the entries
    uint8_t pde = 0;  // pdol entry position
    uint8_t sumPdeResponse = 0;
    byte sendDataTemp[250];
    sendDataTemp[0] = 0x83;
    //sendDataTemp[1] = length of following data , is filled after all data is known
    uint8_t sendDataTempIndex = 2;
    while (pde < pdolLen) {
      bool isOneBytePdolTag = CheckOneBytePdol(pdol[pde]);
      byte byte1 = pdol[pde];
      if (isOneBytePdolTag) {
        // the next byte is the length
        pde++;
        uint8_t respLen = pdol[pde];
        // get the data from the look up and paste it in the sendDataTemp array
        byte resp[respLen];
        byte respLen1;
        bool suc1 = LookUpPdolOneByte(byte1, respLen, resp, &respLen1);
        for (uint8_t i = 0; i < respLen1; i++) {
          sendDataTemp[sendDataTempIndex] = resp[i];
          sendDataTempIndex++;
        }
        sumPdeResponse += respLen;
        pde++;
      } else {
        // two tag bytes, then the length
        //byte byte1 = pdol[pde]; // already catched up
        pde++;
        byte byte2 = pdol[pde];
        pde++;
        uint8_t respLen = pdol[pde];
        // get the data from the look up and paste it in the sendDataTemp array
        byte resp[respLen];
        byte respLen1;
        bool suc2 = LookUpPdolTwoByte(byte1, byte2, respLen, resp, &respLen1);
        for (uint8_t i = 0; i < respLen1; i++) {
          sendDataTemp[sendDataTempIndex] = resp[i];
          sendDataTempIndex++;
        }
        sumPdeResponse += respLen;
        pde++;
      }
    }
    sendDataTemp[1] = sumPdeResponse;  // length of following data
    if (METHOD_DEBUG_PRINT) Serial.printf("Sum requested response bytes: %d\n", sumPdeResponse);

    backLen = 255;
    leByte = 0xF8;
    statusCode = SendPdol_Le(sendDataTemp, sumPdeResponse + 2, leByte, backData, &backLen);

    if (backLen == 2) {
      if (METHOD_DEBUG_PRINT) Serial.printf("statusCode %d backLen %d\n", statusCode, backLen);
      if (backData[backLen - 2] == 0x67 && backData[backLen - 1] == 0x00) {
        // this means the card is asking for a 'zero' Le
        if (METHOD_DEBUG_PRINT) Serial.println("Card is asking for Le = 0x00");
        backLen = 255;
        leByte = 0x00;
        statusCode = SendPdol_Le(sendDataTemp, sumPdeResponse + 2, leByte, backData, &backLen);
      }
    }
  }

  if (METHOD_DEBUG_PRINT) Serial.printf("SendPdol statusCode %02x\n", statusCode);
  if (statusCode != EMV_STATUS_OK) {
    Serial.println("SendPdol statusCode ERROR - no more decoding");
    return EMV_STATUS_ERROR;
  }

  // BER-TLV decoder
  uint8_t buffer[255];
  //TLVS tlvs;
  TLVNode *tlvNode, *childNode;
  size_t data_size;
  memcpy(buffer, backData, backLen);
  tlvs.decodeTLVs(buffer, sizeof(buffer));

  int tlvsErrorValue = tlvs.errorValue();
  //Serial.printf("tlvsErrorValue %d\n", tlvsErrorValue);
  /*
    int tlvNodeErrorCodes = tlvNode->errorCodes();
    Serial.printf("tlvNodeErrorCodes %d\n", tlvNodeErrorCodes);
    int tlvChildNodeErrorCodes = childNode->errorCodes();
    Serial.printf("tlvChildNodeErrorCodes %d\n", tlvChildNodeErrorCodes);
    */

  // Dump the decoded TLV structure
  tlvNode = tlvs.firstTLV();
  if (METHOD_DEBUG_PRINT) {
    Serial.print("TLV Node ");
    Serial.println(tlvNode->getTag(), HEX);
  }
  for (childNode = tlvNode->firstChild(); childNode; childNode = tlvNode->nextChild(childNode)) {
    if (METHOD_DEBUG_PRINT) {
      Serial.print("Child Node ");
      Serial.println(childNode->getTag(), HEX);
    }
  }
  TLVS::printTLV(tlvNode);

  // search for tag 57 Track 2 Equivalent Data
  if (METHOD_DEBUG_PRINT) Serial.printf("Search for tag 57 (Track 2 Equivalent Data)\n");
  // find a tag
  // TLVNode* findTLV(uint16_t tag);
  // TLVNode* findNextTLV(TLVNode* node);
  tag57CompleteLen = 255;
  TLVNode* tlvNodeSearch;
  uint16_t tag57 = 0x57;
  tlvNodeSearch = tlvs.findTLV(tag57);

  // don't proceed if result is NULL

  if (tlvNodeSearch != NULL) {

    const uint8_t* tag57Value = tlvNodeSearch->getValue();
    uint32_t tag57ValueLength = tlvNodeSearch->getValueLength();

    if (METHOD_DEBUG_PRINT) Serial.printf("tlvNode->getValue length %d\n", tag57ValueLength);
    //printHex((byte) tag4FValue, sizeof(tag4FValue));
    //for (uint8_t i = 0; i < sizeof(tag4FValue); i++) {
    for (uint8_t i = 0; i < tag57ValueLength; i++) {
      if (METHOD_DEBUG_PRINT) Serial.printf("%02x ", tag57Value[i]);
      tag57Complete[i] = tag57Value[i];
    }
    tag57CompleteLen = tag57ValueLength;

    // get the pan and exp date
    t57PanLen = 0;
    t57ExpDateLen = 0;
    bool isPanDelimiterFound = false;
    byte posByte;
    uint8_t posIndex = 0;
    //char panChar[30];
    memset(panChar, 0, 20);
    //uint8_t panCharLen = 0;
    char bChar[2];
    //char expDateChar[10];
    //uint8_t expDateCharLen = 0;
    while (!isPanDelimiterFound) {
      posByte = tag57Complete[posIndex];
      byte upperByte = (tag57Complete[posIndex] & 0xF0) >> 4;
      byte lowerByte = (tag57Complete[posIndex] & 0x0F);
      if (METHOD_DEBUG_PRINT) Serial.printf("posIndex %d byte %02x upperByte %02x lowerByte %02x\n", posIndex, tag57Complete[posIndex], upperByte, lowerByte);
      if (upperByte != 0xd) {
        sprintf(bChar, "%x", upperByte);
        if (posIndex == 0) {
          strcpy(panChar, bChar);
        } else {
          strcat(panChar, bChar);
        }
        panCharLen++;
        if (lowerByte != 0xd) {
          sprintf(bChar, "%x", lowerByte);
          strcat(panChar, bChar);
          panCharLen++;
        }
      }
      if ((upperByte == 0xd) || (lowerByte == 0xd)) {
        if (upperByte == 0xd) {
          // the lower byte contains the first expiring year
          sprintf(bChar, "%x", lowerByte);
          strcpy(expDateChar, bChar);
          expDateCharLen++;
        }
        isPanDelimiterFound = true;
      }
      posIndex++;
    }

    // now we copy 1..4 expiring date characters
    while (expDateCharLen < 4) {
      posByte = tag57Complete[posIndex];
      byte upperByte = (tag57Complete[posIndex] & 0xF0) >> 4;
      byte lowerByte = (tag57Complete[posIndex] & 0x0F);
      if (METHOD_DEBUG_PRINT) Serial.printf("posIndex %d byte %02x upperByte %02x lowerByte %02x\n", posIndex, tag57Complete[posIndex], upperByte, lowerByte);
      sprintf(bChar, "%x", upperByte);
      if (expDateCharLen == 0) {
        strcpy(expDateChar, bChar);
      } else {
        strcat(expDateChar, bChar);
      }
      expDateCharLen++;
      if (expDateCharLen < 4) {
        sprintf(bChar, "%x", lowerByte);
        strcat(expDateChar, bChar);
        expDateCharLen++;
      }
      posIndex++;
    }
    if (METHOD_DEBUG_PRINT) {
      Serial.printf("Pan length %d: %s\n", panCharLen, panChar);
      Serial.printf("ExpDate length %d: %s\n", expDateCharLen, expDateChar);
    }
  } else {
    if (METHOD_DEBUG_PRINT) Serial.println("No tag57 found");
  }

  // search for tag 94h = AFL = Application File Locator
  if (METHOD_DEBUG_PRINT) Serial.printf("Search for tag 94 (AFL Application File Locator)\n");
  bool tag94Found = false;
  t94AflLen = 0;
  uint16_t tag94 = 0x94;
  tlvNodeSearch = tlvs.findTLV(tag94);
  if (tlvNodeSearch != NULL) {
    tag94Found = true;
    const uint8_t* tag94Value = tlvNodeSearch->getValue();
    uint32_t tag94ValueLength = tlvNodeSearch->getValueLength();

    if (METHOD_DEBUG_PRINT) Serial.printf("tlvNode->getValue length %d\n", tag94ValueLength);
    //printHex((byte) tag4FValue, sizeof(tag4FValue));
    //for (uint8_t i = 0; i < sizeof(tag4FValue); i++) {
    for (uint8_t i = 0; i < tag94ValueLength; i++) {
      if (METHOD_DEBUG_PRINT) Serial.printf("%02x ", tag94Value[i]);
      t94Afl[i] = tag94Value[i];
    }
    if (METHOD_DEBUG_PRINT) Serial.println();
    t94AflLen = tag94ValueLength;
  } else {
    if (METHOD_DEBUG_PRINT) Serial.println("No tag94 (AFL) found");
  }

  // now search for 'Response Message Template Format 1' that is in use e.g. for American Express Cards
  // search for tag 80h = Response Message Template Format 1
  if (!tag94Found) {
    if (METHOD_DEBUG_PRINT) Serial.printf("Search for tag 80h (Response Message Template Format 1)\n");
    uint8_t t80Len = 0;
    uint16_t tag80 = 0x80;
    byte t80[250];
    tlvNodeSearch = tlvs.findTLV(tag80);
    if (tlvNodeSearch != NULL) {
      if (METHOD_DEBUG_PRINT) Serial.println("Found Tag 80");
      const uint8_t* tag80Value = tlvNodeSearch->getValue();
      uint32_t tag80ValueLength = tlvNodeSearch->getValueLength();

      if (METHOD_DEBUG_PRINT) Serial.printf("tlvNode->getValue length %d\n", tag80ValueLength);
      //printHex((byte) tag4FValue, sizeof(tag4FValue));
      //for (uint8_t i = 0; i < sizeof(tag4FValue); i++) {
      for (uint8_t i = 0; i < tag80ValueLength; i++) {
        if (METHOD_DEBUG_PRINT) Serial.printf("%02x ", tag80Value[i]);
        t80[i] = tag80Value[i];
      }
      if (METHOD_DEBUG_PRINT) Serial.println();
      t80Len = tag80ValueLength;

      // I'm reusing the wrong variable
      for (uint8_t i = 0; i < t80Len - 2; i++) {
        t94Afl[i] = t80[i + 2];
      }
      t94AflLen = t80Len - 2;
    } else {
      if (METHOD_DEBUG_PRINT) Serial.println("No tag80 (Response Message Template Format 1) found");
    }
  }

  return EMV_STATUS_OK;
}

// This is the native code for SendPdol that allows for a flexible Le byte
ESP32_EMV::EMV_StatusCode ESP32_EMV::SendPdol_Le(byte* sendData, byte sendLen, byte leByte, byte* backReadData, uint16_t* backReadLen) {
  if (METHOD_DEBUG_PRINT) {
    Serial.printf("SendPdol leByte %02x sendLen %d data:\n", leByte, sendLen);
    printHex(sendData, sendLen);
    Serial.println();
  }

  byte sendData2[sendLen + 6];
  sendData2[0] = 0x80;           // CLA
  sendData2[1] = 0xA8;           // INS
  sendData2[2] = 0x00;           // P1
  sendData2[3] = 0x00;           // P2
  sendData2[4] = (byte)sendLen;  // Lc
  memcpy(&sendData2[5], sendData, sendLen);
  sendData2[sendLen + 5] = leByte;  // Le

  byte backData[255];
  byte backLen = 255;

  EMV_StatusCode statusCode;

  statusCode = EMV_BasicTransceive(sendData2, sendLen + 6, backData, &backLen);
  memcpy(backReadData, backData, backLen);
  *backReadLen = backLen;
  return statusCode;
}

bool ESP32_EMV::LookUpPdolOneByte(byte byte1, byte length, byte* resData, byte* resLength) {
  if (byte1 == 0x95) {
    uint8_t rLen = 5;
    byte temp[rLen];
    memset(temp, 0, rLen);
    memcpy(resData, temp, rLen);
    *resLength = rLen;
    if (PDOL_DEBUG_PRINT) {
      Serial.printf("LookUp 1 Byte byte1 %02x byte2 -- length %2d respLen %d resData", byte1, length, rLen);
      printHex(resData, rLen);
      Serial.println();
    }
    return true;
  } else if (byte1 == 0x9a) {
    uint8_t rLen = 3;
    byte temp[rLen];
    //temp[0] = 0x23;
    temp[0] = 0x25;
    temp[1] = 0x03;
    temp[2] = 0x01;
    memcpy(resData, temp, rLen);
    *resLength = rLen;
    if (PDOL_DEBUG_PRINT) {
      Serial.printf("LookUp 1 Byte byte1 %02x byte2 -- length %2d respLen %d resData", byte1, length, rLen);
      printHex(resData, rLen);
      Serial.println();
    }
    return true;
  } else if (byte1 == 0x9c) {
    uint8_t rLen = 1;
    byte temp[rLen];
    temp[0] = 0x00;
    memcpy(resData, temp, rLen);
    *resLength = rLen;
    if (PDOL_DEBUG_PRINT) {
      Serial.printf("LookUp 1 Byte byte1 %02x byte2 -- length %2d respLen %d resData", byte1, length, rLen);
      printHex(resData, rLen);
      Serial.println();
    }
    return true;
  } else {
    byte temp[length];
    memset(temp, 0, length);
    memcpy(resData, temp, length);
    *resLength = length;
    if (PDOL_DEBUG_PRINT) {
      Serial.printf("LookUp 1 Byte byte1 %02x byte2 -- length %2d respLen %d resData", byte1, length, length);
      printHex(resData, length);
      Serial.println();
    }
    return false;
  }
}

bool ESP32_EMV::LookUpPdolTwoByte(byte byte1, byte byte2, byte length, byte* resData, byte* resLength) {
  if ((byte1 == 0x9f) && (byte2 == 0x66)) {
    uint8_t rLen = 4;
    byte temp[rLen];
    memset(temp, 0, rLen);
    temp[0] = 0x27;
    memcpy(resData, temp, rLen);
    *resLength = rLen;
    if (PDOL_DEBUG_PRINT) {
      Serial.printf("LookUp 2 Byte byte1 %02x byte2 %02x length %2d respLen %d resData", byte1, byte2, length, rLen);
      printHex(resData, rLen);
      Serial.println();
    }
    return true;
  } else if ((byte1 == 0x9f) && (byte2 == 0x02)) {
    uint8_t rLen = 6;
    byte temp[rLen];
    memset(temp, 0, rLen);
    temp[4] = 0x10;
    memcpy(resData, temp, rLen);
    *resLength = rLen;
    if (PDOL_DEBUG_PRINT) {
      Serial.printf("LookUp 2 Byte byte1 %02x byte2 %02x length %2d respLen %d resData", byte1, byte2, length, rLen);
      printHex(resData, rLen);
      Serial.println();
    }
    return true;
  } else if ((byte1 == 0x9f) && (byte2 == 0x03)) {
    uint8_t rLen = 6;
    byte temp[rLen];
    memset(temp, 0, rLen);
    memcpy(resData, temp, rLen);
    *resLength = rLen;
    if (PDOL_DEBUG_PRINT) {
      Serial.printf("LookUp 2 Byte byte1 %02x byte2 %02x length %2d respLen %d resData", byte1, byte2, length, rLen);
      printHex(resData, rLen);
      Serial.println();
    }
    return true;
  } else if ((byte1 == 0x9f) && (byte2 == 0x1a)) {
    uint8_t rLen = 2;
    byte temp[rLen];
    temp[0] = 0x09;
    temp[1] = 0x78;
    memcpy(resData, temp, rLen);
    *resLength = rLen;
    if (PDOL_DEBUG_PRINT) {
      Serial.printf("LookUp 2 Byte byte1 %02x byte2 %02x length %2d respLen %d resData", byte1, byte2, length, rLen);
      printHex(resData, length);
      Serial.println();
    }
    return true;
  } else if ((byte1 == 0x5f) && (byte2 == 0x2a)) {
    uint8_t rLen = 2;
    byte temp[rLen];
    temp[0] = 0x09;
    temp[1] = 0x78;
    memcpy(resData, temp, rLen);
    *resLength = rLen;
    if (PDOL_DEBUG_PRINT) {
      Serial.printf("LookUp 2 Byte byte1 %02x byte2 %02x length %2d respLen %d resData", byte1, byte2, length, rLen);
      printHex(resData, rLen);
      Serial.println();
    }
    return true;
  } else if ((byte1 == 0x9f) && (byte2 == 0x37)) {
    uint8_t rLen = 4;
    byte temp[rLen];
    temp[0] = 0x38;
    temp[1] = 0x39;
    temp[2] = 0x30;
    temp[3] = 0x31;
    memcpy(resData, temp, rLen);
    *resLength = rLen;
    if (PDOL_DEBUG_PRINT) {
      Serial.printf("LookUp 2 Byte byte1 %02x byte2 %02x length %2d respLen %d resData", byte1, byte2, length, rLen);
      printHex(resData, rLen);
      Serial.println();
    }
    return true;
  } else if ((byte1 == 0x9f) && (byte2 == 0x35)) {
    uint8_t rLen = 1;
    byte temp[rLen];
    temp[0] = 0x22;
    memcpy(resData, temp, rLen);
    *resLength = rLen;
    if (PDOL_DEBUG_PRINT) {
      Serial.printf("LookUp 2 Byte byte1 %02x byte2 %02x length %2d respLen %d resData", byte1, byte2, length, rLen);
      printHex(resData, rLen);
      Serial.println();
    }
    return true;
  } else if ((byte1 == 0x9f) && (byte2 == 0x45)) {
    uint8_t rLen = 2;
    byte temp[rLen];
    temp[0] = 0x00;
    temp[1] = 0x00;
    memcpy(resData, temp, rLen);
    *resLength = rLen;
    if (PDOL_DEBUG_PRINT) {
      Serial.printf("LookUp 2 Byte byte1 %02x byte2 %02x length %2d respLen %d resData", byte1, byte2, length, rLen);
      printHex(resData, rLen);
      Serial.println();
    }
    return true;
  } else if ((byte1 == 0x9f) && (byte2 == 0x4c)) {
    uint8_t rLen = 8;
    byte temp[rLen];
    memset(temp, 0, rLen);
    memcpy(resData, temp, rLen);
    *resLength = rLen;
    if (PDOL_DEBUG_PRINT) {
      Serial.printf("LookUp 2 Byte byte1 %02x byte2 %02x length %2d respLen %d resData", byte1, byte2, length, rLen);
      printHex(resData, rLen);
      Serial.println();
    }
    return true;
  } else if ((byte1 == 0x9f) && (byte2 == 0x34)) {
    uint8_t rLen = 3;
    byte temp[rLen];
    memset(temp, 0, rLen);
    memcpy(resData, temp, rLen);
    *resLength = rLen;
    if (PDOL_DEBUG_PRINT) {
      Serial.printf("LookUp 2 Byte byte1 %02x byte2 %02x length %2d respLen %d resData", byte1, byte2, length, rLen);
      printHex(resData, rLen);
      Serial.println();
    }
    return true;
  } else if ((byte1 == 0x9f) && (byte2 == 0x21)) {
    uint8_t rLen = 3;
    byte temp[rLen];
    temp[0] = 0x11;
    temp[1] = 0x10;
    temp[2] = 0x09;
    memcpy(resData, temp, rLen);
    *resLength = rLen;
    if (PDOL_DEBUG_PRINT) {
      Serial.printf("LookUp 2 Byte byte1 %02x byte2 %02x length %2d respLen %d resData", byte1, byte2, length, rLen);
      printHex(resData, rLen);
      Serial.println();
    }
    return true;
  } else if ((byte1 == 0x9f) && (byte2 == 0x7c)) {
    uint8_t rLen = 14;
    byte temp[rLen];
    memset(temp, 0, rLen);
    memcpy(resData, temp, rLen);
    *resLength = rLen;
    if (PDOL_DEBUG_PRINT) {
      Serial.printf("LookUp 2 Byte byte1 %02x byte2 %02x length %2d respLen %d resData", byte1, byte2, length, rLen);
      printHex(resData, rLen);
      Serial.println();
    }
    return true;
  } else {
    byte temp[length];
    memset(temp, 0, length);
    memcpy(resData, temp, length);
    *resLength = length;
    if (PDOL_DEBUG_PRINT) Serial.printf("LookUp 2 Byte byte1 %02x byte2 -- length %2d respLen %d resData", byte1, length, length);
    return false;
  }
}

// returns true if data byte is in the list of one byte commands
bool ESP32_EMV::CheckOneBytePdol(byte data) {
  // checks a list against the byte https://paymentcardtools.com/emv-misc/dols
  if (data == 0x95) {
    return true;
  } else if (data == 0x9a) {
    return true;
  } else if (data == 0x9c) {
    return true;
  }
  return false;
}

ESP32_EMV::EMV_StatusCode ESP32_EMV::ReadRecord(byte* aflEntry, byte* appData, uint16_t* backReadLen) {
  if (METHOD_DEBUG_PRINT) {
    Serial.print("ReadRecord");
    printHex(aflEntry, 4);
    Serial.println();
  }

  // https://werner.rothschopf.net/201703_arduino_esp8266_nfc.htm
  uint8_t SFI = aflEntry[0] >> 3;
  uint8_t P2 = SFI << 3 | 0b00000100;
/*
  byte sendData[5];
  sendData[0] = 0x00;           // Class
  sendData[1] = 0xB2;           // CMD
  sendData[2] = aflChunked[1];  // P1
  sendData[3] = P2;             // P2
  //sendData[4] = 0x00;          // Le
  sendData[4] = 0xF8;  // Le // works fine for MC Openbank first 3 files
*/
  byte backData[255];
  byte backLen = 255;
  byte leByte = 0xF8;
  EMV_StatusCode statusCode;

  //statusCode = EMV_BasicTransceive(sendData, sizeof(sendData), backData, &backLen);
  statusCode = ReadRecord_Le(aflEntry, leByte, backData, &backLen);

  if (backLen == 2) {
    if (METHOD_DEBUG_PRINT) Serial.printf("statusCode %d backLen %d\n", statusCode, backLen);
    if (backData[backLen - 2] == 0x67 && backData[backLen - 1] == 0x00) {
      // this means the card is asking for a 'zero' Le
      Serial.println("Card is asking for Le = 0x00");
      backLen = 255;
      leByte = 0x00;
      statusCode = ReadRecord_Le(aflEntry, leByte, backData, &backLen);
    }
  }

  if (METHOD_DEBUG_PRINT) Serial.printf("*** ReadRecord backLen %d\n", backLen);
  if (backLen == 0) {
    if (METHOD_DEBUG_PRINT) Serial.println("Received no valid response, aborting");
    *backReadLen = 255;
    memcpy(appData, backData, backLen);
    return EMV_STATUS_NO_RESPONSE;
  }

  // BER-TLV decoder
  uint8_t buffer[255];
  //TLVS tlvs;
  TLVNode *tlvNode, *childNode;
  size_t data_size;
  memcpy(buffer, backData, backLen);
  tlvs.decodeTLVs(buffer, sizeof(buffer));

  int tlvsErrorValue = tlvs.errorValue();
  //Serial.printf("tlvsErrorValue %d\n", tlvsErrorValue);
  /*
    int tlvNodeErrorCodes = tlvNode->errorCodes();
    Serial.printf("tlvNodeErrorCodes %d\n", tlvNodeErrorCodes);
    int tlvChildNodeErrorCodes = childNode->errorCodes();
    Serial.printf("tlvChildNodeErrorCodes %d\n", tlvChildNodeErrorCodes);
    */

  // Dump the decoded TLV structure
  tlvNode = tlvs.firstTLV();
  if (METHOD_DEBUG_PRINT) {
    Serial.print("TLV Node ");
    Serial.println(tlvNode->getTag(), HEX);
  }
  for (childNode = tlvNode->firstChild(); childNode; childNode = tlvNode->nextChild(childNode)) {
    if (METHOD_DEBUG_PRINT) {
      Serial.print("Child Node ");
      Serial.println(childNode->getTag(), HEX);
    }
  }
  TLVS::printTLV(tlvNode);

  // find Tag5A (PAN) and Tag5F24 (Exp.Date)
  /*
  byte t5aPan[20];
  byte t5aPanLen = 0;
  byte t5f24ExpDate[10];
  byte t5f24ExpDateLen = 0; // 0 = not found
*/

  // search for tag 57 Track 2 Equivalent Data
  // find a tag
  // TLVNode* findTLV(uint16_t tag);
  // TLVNode* findNextTLV(TLVNode* node);
  t5aPanLen = 0;
  t5f24ExpDateLen = 0;

  TLVNode* tlvNodeSearch;
  uint16_t tag5a = 0x5A;
  tlvNodeSearch = tlvs.findTLV(tag5a);

  // don't proceed if result is NULL

  if (tlvNodeSearch != NULL) {

    const uint8_t* tag5aValue = tlvNodeSearch->getValue();
    uint32_t tag5aValueLength = tlvNodeSearch->getValueLength();

    if (METHOD_DEBUG_PRINT) Serial.printf("tlvNode->getValue length %d\n", tag5aValueLength);
    //printHex((byte) tag4FValue, sizeof(tag4FValue));
    for (uint8_t i = 0; i < tag5aValueLength; i++) {
      if (METHOD_DEBUG_PRINT) Serial.printf("%02x ", tag5aValue[i]);
      t5aPan[i] = tag5aValue[i];
    }
    t5aPanLen = tag5aValueLength;
    if (METHOD_DEBUG_PRINT) Serial.printf("PAN found length %d\n", t5aPanLen);
    printHex(t5aPan, t5aPanLen);
    Serial.println();
  }

  // search for tag5f24 exp. date
  uint16_t tag5f24 = 0x5f24;
  tlvNodeSearch = tlvs.findTLV(tag5f24);

  // don't proceed if result is NULL

  if (tlvNodeSearch != NULL) {

    const uint8_t* tag5f24Value = tlvNodeSearch->getValue();
    uint32_t tag5f24ValueLength = tlvNodeSearch->getValueLength();

    if (METHOD_DEBUG_PRINT) Serial.printf("tlvNode->getValue length %d\n", tag5f24ValueLength);
    for (uint8_t i = 0; i < tag5f24ValueLength; i++) {
      if (METHOD_DEBUG_PRINT) Serial.printf("%02x ", tag5f24Value[i]);
      t5f24ExpDate[i] = tag5f24Value[i];
    }
    t5f24ExpDateLen = tag5f24ValueLength;
    if (METHOD_DEBUG_PRINT) {
      Serial.printf("Expire Date found length %d\n", t5f24ExpDateLen);
      printHex(t5f24ExpDate, t5f24ExpDateLen);
      Serial.println();
    }
  }

  *backReadLen = backLen;
  memcpy(appData, backData, backLen);
  return EMV_STATUS_OK;
}

ESP32_EMV::EMV_StatusCode ESP32_EMV::ReadRecord_Le(byte* aflEntry, byte leByte, byte* backReadData, byte* backReadLen) {
  if (METHOD_DEBUG_PRINT) {
    Serial.print("ReadRecord_Le");
    printHex(aflEntry, 4);
    Serial.println();
  }

  // https://werner.rothschopf.net/201703_arduino_esp8266_nfc.htm
  uint8_t SFI = aflEntry[0] >> 3;
  uint8_t P2 = SFI << 3 | 0b00000100;

  byte sendData[5];
  sendData[0] = 0x00;        // Class
  sendData[1] = 0xB2;        // CMD
  sendData[2] = aflEntry[1]; // P1
  sendData[3] = P2;          // P2
  sendData[4] = leByte;      // Le
  byte backData[255];
  byte backLen = 255;
  EMV_StatusCode statusCode;

  statusCode = EMV_BasicTransceive(sendData, sizeof(sendData), backData, &backLen);
  memcpy(backReadData, backData, backLen);
  *backReadLen = backLen;
  return statusCode;
}

// This is a very simplyfied table with my most used Credit Card issuers. It returns true if
// an AID was found or false if it is an unknown AID
// see https://www.eftlab.com/knowledge-base/complete-list-of-application-identifiers-aid
// for a complete list of AIDs
ESP32_EMV::EMV_StatusCode ESP32_EMV::LookUpAid(byte* sendData, byte sendLen, uint8_t* aidNameIndex) {
  // A0 00 00 00 03 (10) Visa
  // A0 00 00 00 04 (10) MasterCard
  // A0 00 00 00 25 (10) American Express
  // A0 00 00 00 59 (10) German girocard
  if (sendLen < 5) return EMV_STATUS_ERROR;
  if (sendData[0] != 0xA0) return EMV_STATUS_ERROR;
  if (sendData[1] != 0x00) return EMV_STATUS_ERROR;
  if (sendData[2] != 0x00) return EMV_STATUS_ERROR;
  if (sendData[3] != 0x00) return EMV_STATUS_ERROR;
  if (sendData[4] == 0x03) {
    *aidNameIndex = 1;
    Serial.println("VisaC");
    return EMV_STATUS_OK;
  } else if (sendData[4] == 0x04) {
    *aidNameIndex = 2;
    Serial.println("MasterC");
    return EMV_STATUS_OK;
  } else if (sendData[4] == 0x25) {
    *aidNameIndex = 3;
    Serial.println("AmexCo");
    return EMV_STATUS_OK;
  } else if (sendData[4] == 0x59) {
    *aidNameIndex = 4;
    Serial.println("giroC");
    return EMV_STATUS_OK;
  } else {
    *aidNameIndex = 0;
    Serial.println("UNKNOWN");
    return EMV_STATUS_ERROR;
  }
}

// PROTECTED

/*
Hexstring to byte array helper
https://forum.arduino.cc/t/hex-string-to-byte-array/563827/3
usage:
byte byteArray[MaxByteArraySize] = {0};
hexCharacterStringToBytes(byteArray, "A489B1");
dumpByteArray(byteArray, MaxByteArraySize);

*/
void ESP32_EMV::hexCharacterStringToBytes(byte* byteArray, const char* hexString) {
  bool oddLength = strlen(hexString) & 1;

  byte currentByte = 0;
  byte byteIndex = 0;

  for (byte charIndex = 0; charIndex < strlen(hexString); charIndex++) {
    bool oddCharIndex = charIndex & 1;

    if (oddLength) {
      // If the length is odd
      if (oddCharIndex) {
        // odd characters go in high nibble
        currentByte = nibble(hexString[charIndex]) << 4;
      } else {
        // Even characters go into low nibble
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    } else {
      // If the length is even
      if (!oddCharIndex) {
        // Odd characters go into the high nibble
        currentByte = nibble(hexString[charIndex]) << 4;
      } else {
        // Odd characters go into low nibble
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
  }
}

byte ESP32_EMV::nibble(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';

  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  return 0;  // Not a valid hexadecimal character
}

void ESP32_EMV::dumpByteArray(const byte* byteArray, const byte arraySize) {
  for (int i = 0; i < arraySize; i++) {
    Serial.print("0x");
    if (byteArray[i] < 0x10)
      Serial.print("0");
    Serial.print(byteArray[i], HEX);
    Serial.print(", ");
  }
  Serial.println();
}

void ESP32_EMV::convertLargeInt2Uint8_t4Lsb(int& input, uint8_t* output) {
  int max2Byte = 65536;
  int highInt = input / max2Byte;
  int lowInt = input % max2Byte;
  uint8_t highUint8[2];
  convertInt2Uint8_t(highInt, highUint8);
  uint8_t lowUint8[2];
  convertInt2Uint8_t(lowInt, lowUint8);
  uint8_t convertedInt[4];
  convertedInt[3] = highUint8[0];
  convertedInt[2] = highUint8[1];
  convertedInt[1] = lowUint8[0];
  convertedInt[0] = lowUint8[1];
  int i;
  for (i = 0; i < 4; i++) {
    output[i] = convertedInt[i];
  }
}

// maximum int is 65535
void ESP32_EMV::convertInt2Uint8_t(int& input, uint8_t* output) {
  byte high = highByte(input);
  byte low = lowByte(input);
  //uint8_t uint8_22[2];
  output[0] = high;
  output[1] = low;
}

int ESP32_EMV::convertUint8_t3_2IntLsb(byte* input) {
  int outputValue = 0;
  outputValue += input[0];
  outputValue += (input[1] * 256);
  outputValue += (input[2] * 65536);
  //output = outputValue;
  return outputValue;
}

int ESP32_EMV::convertUint8_t4_2IntLsb(byte* input) {
  int outputValue = 0;
  outputValue += input[0];
  outputValue += (input[1] * 256);
  outputValue += (input[2] * 65536);
  outputValue += (input[3] * 16777216);
  return outputValue;
}

void ESP32_EMV::convertIntTo3BytesLsb(int input, byte* output) {
  byte* out = new byte[3];
  if (input > 16777215) {
    memset(output, 0, 3);
    //memcpy(output, out, 3);
    return;
  }
  out[0] = input & 0xff;
  out[1] = (input >> 8) & 0xff;
  out[2] = (input >> 16) & 0xff;
  memcpy(output, out, 3);
}

byte ESP32_EMV::upperPartByte(byte data) {
  return 0;
}

byte ESP32_EMV::lowerPartByte(byte data) {
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
//
// Protected functions
//
/////////////////////////////////////////////////////////////////////////////////////

ESP32_EMV::EMV_StatusCode ESP32_EMV::EMV_BasicTransceive(byte* sendData, byte sendLen, byte* backData, byte* backLen) {
  ESP32_EMV::EMV_StatusCode result;
  bool success;
  EMV_StatusCode statusCode;
  byte bLen = 255;
  if (COMM_DEBUG_PRINT) {
    Serial.printf("Send length %d\n", sendLen);
    printHex(sendData, sendLen);
    Serial.println("");
  }
  success = emvLib->inDataExchange(sendData, sendLen, backData, &bLen);
  if (COMM_DEBUG_PRINT) {
    Serial.printf("Recv length %d\n", bLen);
    printHex(backData, bLen);
    Serial.println("");
  }
  if (success) {
    if (bLen == 255) {
      *backLen = bLen;
      return EMV_STATUS_NO_RESPONSE;
    } else {
      *backLen = bLen;
      return EMV_STATUS_OK;
    }
  } else {
    *backLen = 0;
    return EMV_STATUS_ERROR;
  }
}

/////////////////////////////////////////////////////////////////////////////////////
//
// Internal management
//
/////////////////////////////////////////////////////////////////////////////////////

void ESP32_EMV::printHex(byte* buffer, uint16_t bufferSize) {
  for (uint16_t i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
