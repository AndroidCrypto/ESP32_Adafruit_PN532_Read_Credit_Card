
void run_E01_Credit_Card_Handling() {
  Serial.println();
  Serial.println(DIVIDER);
  Serial.println(" E01 Credit Card Handling");
  Serial.println(DIVIDER);
  delay(100);
  Serial.println(DIVIDER);

  Serial.println(DIVIDER);
  Serial.println("Select PPSE");
  byte* appData = new byte[255];
  uint16_t appLenExt = 255;
  ESP32_EMV::EMV_StatusCode emvStatusCode;
  emvStatusCode = emv.SelectPpse(appData, &appLenExt);
  Serial.printf("Sel PPSE %02x appLenExt %d\n", emvStatusCode, appLenExt);

  if (emvStatusCode != ESP32_EMV::EMV_STATUS_OK) {
    Serial.println("Error Select PPSE, aborting");
    return;
  }

  Serial.println(DIVIDER);
  Serial.println("Get AIDs from response");
  // 10 aids of max 16 bytes length

  Serial.printf("Found %d AIDs\n", emv.numberOfAids);
  for (uint8_t i = 0; i < emv.numberOfAids; i++) {
    Serial.printf("AID %d: ", i + 1);
    emv.printHex(emv.aids[i], emv.aidsLen[i]);
    Serial.println();
  }

  // iterate through AIDs to get the PDOL for each AID
  for (uint8_t aidIndex = 0; aidIndex - emv.numberOfAids; aidIndex++) {
    Serial.println(DIVIDER);
    Serial.printf("AID %d:", aidIndex + 1);
    emv.printHex(emv.aids[aidIndex], emv.aidsLen[aidIndex]);
    Serial.println();
    // aidLookUp is a very simplyfied table for my most used AIDs

    uint8_t aidNameIndex;
    emvStatusCode = emv.LookUpAid(emv.aids[aidIndex], emv.aidsLen[aidIndex], &aidNameIndex);
    if (emvStatusCode == ESP32_EMV::EMV_STATUS_OK) {
      if (aidNameIndex == 1) {
        Serial.println("VisaCard");
      } else if (aidNameIndex == 2) {
        Serial.println("MasterCd");
      } else if (aidNameIndex == 3) {
        Serial.println("AmexCoCd");
      } else if (aidNameIndex == 4) {
        Serial.println("giroCard");
      } else {
        Serial.println("UKNOWN CreditCard");
      }
    } else {
      Serial.println("UKNOWN CreditCard");
    }

    Serial.println("Select AID");
    appLenExt = 255;
    memset(appData, 0, appLenExt);
    emvStatusCode = emv.SelectApdu(emv.aids[aidIndex], emv.aidsLen[aidIndex], 0x02, appData, &appLenExt);
    // for the next step we need to know if the card requested a PDOL (tag 9F38 in response
    if (emv.pdolLen > 254) {
      Serial.println("No PDOL found in response, using a nulled PDOL");
      // now contruct a pdol
      appLenExt = 255;
      memset(appData, 0, appLenExt);
      emvStatusCode = emv.SendPdol(appData, &appLenExt);
    } else {
      Serial.printf("Found PDOLs (len %d):", emv.pdolLen);
      emv.printHex(emv.pdol, emv.pdolLen);
      Serial.println();
      // now contruct a pdol
      appLenExt = 255;
      memset(appData, 0, appLenExt);
      emvStatusCode = emv.SendPdol(appData, &appLenExt);

      if (emvStatusCode != ESP32_EMV::EMV_STATUS_OK) {
        Serial.println("Error Send PDOL, aborting");
        return;
      }

      if (emv.panCharLen > 0) {
        char panCharMask[5];
        memset(panCharMask, 0, 5);
        for (uint8_t i = 0; i < 4; i++) {
          panCharMask[i] = emv.panChar[i];
        }
        Serial.printf("PAN %s", panCharMask);
        Serial.println(" ****");
        Serial.print("ExpDate ");
        Serial.printf("%s\n", emv.expDateChar);
      }
    }  // selectApdu if (desfire.pdolLen > 254)

    // read the AFL
    // https://werner.rothschopf.net/201703_arduino_esp8266_nfc.htm

    Serial.println(DIVIDER);
    Serial.println("AFL Handling");
    if (emv.t94AflLen == 0) {
      Serial.println("No AFL found");
    } else {
      Serial.printf("AFL length %d\n", emv.t94AflLen);

      uint8_t numberOfAfl = emv.t94AflLen / 4;
      Serial.printf("Number of AFL entries %d\n", numberOfAfl);

      // chunk in 4 byte chunks
      byte aflEntry[4];

      for (uint8_t j = 0; j < numberOfAfl; j++) {
        Serial.println(DIVIDER);
        Serial.printf("AFL Entry %d\n", j + 1);
        // SFI  start   end   Number of records in data authentication
        // 10 02 04 00
        // Openbank 18 01 03 00 20 01 01 01
        for (uint8_t i = 0; i < 4; i++) {
          aflEntry[i] = emv.t94Afl[i + (4 * j)];
        }
        // more than 1 file ?
        uint8_t fileIndex = aflEntry[2] - aflEntry[1] + 1;
        Serial.printf("Number of files in AFL entry: %d\n", fileIndex);
        Serial.println(DIVIDER);
        for (uint8_t i = 0; i < fileIndex; i++) {
          Serial.printf("AFL for SFI %02x file %02x\n", aflEntry[0], aflEntry[1]);
          emvStatusCode = emv.ReadRecord(aflEntry, appData, &appLenExt);
          if (emvStatusCode != ESP32_EMV::EMV_STATUS_OK) {
            Serial.println("Error Read Record, skipping");
          }
          Serial.println(DIVIDER);
          if (emv.t5aPanLen > 0) {
            Serial.printf("PAN found length %d\n", emv.t5aPanLen);
            emv.printHex(emv.t5aPan, emv.t5aPanLen);
            Serial.println();
            emv.t5aPanLen = 0;
          }
          if (emv.t5f24ExpDateLen > 0) {
            Serial.printf("Exp.Date found length %d\n", emv.t5f24ExpDateLen);
            emv.printHex(emv.t5f24ExpDate, emv.t5f24ExpDateLen);
            Serial.println();
            emv.t5f24ExpDateLen = 0;
          }
          //}
          aflEntry[1]++;
        }
      }
    }
    // delay for next entry
    if (emv.numberOfAids > 1) {
      delay(2000);
    }
  }

  delay(100);
  Serial.println(DIVIDER);
  Serial.println(" E01 Credit Card Handling END");
  Serial.println(DIVIDER);
  Serial.println();
}