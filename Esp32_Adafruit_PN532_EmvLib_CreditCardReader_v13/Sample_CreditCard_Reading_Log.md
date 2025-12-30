*** The used Credit Card is outdated and all data is void ! ***

ESP32 Adafruit_PN532 EMV Library V13
Found chip PN532
Firmware ver. 1.6
ESP32_EMV library version: 13
Waiting for an ISO14443A card
Tag number: 1
Found a card!

-------------------------------------------------------------------------
 E01 Credit Card Handling
-------------------------------------------------------------------------

Select PPSE
SelectApdu searchIndex 01
SelectApdu leByte f8 sendLen 14 data:
 32 50 41 59 2E 53 59 53 2E 44 44 46 30 31
Send length 20
 00 A4 04 00 0E 32 50 41 59 2E 53 59 53 2E 44 44 46 30 31 F8
Recv length 2
 67 00
statusCode 0 backLen 2
------------------------
Card is asking for Le = 0x00
SelectApdu leByte 00 sendLen 14 data:
 32 50 41 59 2E 53 59 53 2E 44 44 46 30 31
Send length 20
 00 A4 04 00 0E 32 50 41 59 2E 53 59 53 2E 44 44 46 30 31 00
Recv length 68
 6F 40 84 0E 32 50 41 59 2E 53 59 53 2E 44 44 46 30 31 A5 2E BF 0C 2B 61 29 4F 07 A0 00 00 00 03 10 10 50 09 56 69 73 61 20 43 61 72 64 87 01 01 9F 0A 08 00 01 05 02 00 00 00 00 BF 63 04 DF 20 01 80 90 00
TLV Node 6F
Child Node 84
Child Node A5
Tag: 6F Length: 40
    Tag: 84 Length: E
        2 P A Y . S Y S . D D F 0 1 
    Tag: A5 Length: 2E
        Tag: BF0C Length: 2B
            Tag: 61 Length: 29
                Tag: 4F Length: 7
                    A0 00 00 00 03 10 10
                Tag: 50 Length: 9
                    V i s a   C a r d 
                Tag: 87 Length: 1
                    01
                Tag: 9F0A Length: 8
                    00 01 05 02 00 00 00 00
                Tag: BF63 Length: 4
                    Tag: DF20 Length: 1
                        80
Search for tag 4F (AIDs on card)
tlvNode->getValue length 7
a0 00 00 00 03 10 10 Found 1 AIDs on the card
Sel PPSE 00 appLenExt 66
-------------------------------------------------------------------------
Get AIDs from response
Found 1 AIDs
AID 1:  A0 00 00 00 03 10 10
-------------------------------------------------------------------------
AID 1: A0 00 00 00 03 10 10
VisaC
Select AID
SelectApdu searchIndex 02
SelectApdu leByte f8 sendLen 7 data:
 A0 00 00 00 03 10 10
Send length 13
 00 A4 04 00 07 A0 00 00 00 03 10 10 F8
Recv length 2
 67 00
statusCode 0 backLen 2
------------------------
Card is asking for Le = 0x00
SelectApdu leByte 00 sendLen 7 data:
 A0 00 00 00 03 10 10
Send length 13
 00 A4 04 00 07 A0 00 00 00 03 10 10 00
Recv length 85
 6F 51 84 07 A0 00 00 00 03 10 10 A5 46 50 09 56 69 73 61 20 43 61 72 64 87 01 01 9F 38 18 9F 66 04 9F 02 06 9F 03 06 9F 1A 02 95 05 5F 2A 02 9A 03 9C 01 9F 37 04 5F 2D 04 64 65 65 6E BF 0C 13 9F 5A 05 31 09 78 02 76 9F 0A 08 00 01 05 02 00 00 00 00 90 00
TLV Node 6F
Child Node 84
Child Node A5
Tag: 6F Length: 51
    Tag: 84 Length: 7
        A0 00 00 00 03 10 10
    Tag: A5 Length: 46
        Tag: 50 Length: 9
            V i s a   C a r d 
        Tag: 87 Length: 1
            01
        Tag: 9F38 Length: 18
            9F 66 04 9F 02 06 9F 03 06 9F 1A 02 95 05 5F 2A 02 9A 03 9C 01 9F 37 04
        Tag: 5F2D Length: 4
            d e e n 
        Tag: BF0C Length: 13
            Tag: 9F5A Length: 5
                31 09 78 02 76
            Tag: 9F0A Length: 8
                00 01 05 02 00 00 00 00
Search for tag 9F38 (PDOLs)
tlvNode->getValue length 24
9f 66 04 9f 02 06 9f 03 06 9f 1a 02 95 05 5f 2a 02 9a 03 9c 01 9f 37 04 *PDOL*
Found PDOLs (len 24): 9F 66 04 9F 02 06 9F 03 06 9F 1A 02 95 05 5F 2A 02 9A 03 9C 01 9F 37 04
SendPdol is requested with length 24
LookUp 2 Byte byte1 9f byte2 66 length  4 respLen 4 resData 27 00 00 00
LookUp 2 Byte byte1 9f byte2 02 length  6 respLen 6 resData 00 00 00 00 10 00
LookUp 2 Byte byte1 9f byte2 03 length  6 respLen 6 resData 00 00 00 00 00 00
LookUp 2 Byte byte1 9f byte2 1a length  2 respLen 2 resData 09 78
LookUp 1 Byte byte1 95 byte2 -- length  5 respLen 5 resData 00 00 00 00 00
LookUp 2 Byte byte1 5f byte2 2a length  2 respLen 2 resData 09 78
LookUp 1 Byte byte1 9a byte2 -- length  3 respLen 3 resData 25 03 01
LookUp 1 Byte byte1 9c byte2 -- length  1 respLen 1 resData 00
LookUp 2 Byte byte1 9f byte2 37 length  4 respLen 4 resData 38 39 30 31
Sum requested response bytes: 33
SendPdol leByte f8 sendLen 35 data:
 83 21 27 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00 09 78 00 00 00 00 00 09 78 25 03 01 00 38 39 30 31
Send length 41
 80 A8 00 00 23 83 21 27 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00 09 78 00 00 00 00 00 09 78 25 03 01 00 38 39 30 31 F8
Recv length 2
 67 00
statusCode 0 backLen 2
Card is asking for Le = 0x00
SendPdol leByte 00 sendLen 35 data:
 83 21 27 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00 09 78 00 00 00 00 00 09 78 25 03 01 00 38 39 30 31
Send length 41
 80 A8 00 00 23 83 21 27 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00 09 78 00 00 00 00 00 09 78 25 03 01 00 38 39 30 31 00
Recv length 219
 77 81 D6 82 02 20 00 94 04 10 01 04 00 57 13 41 63 69 10 02 56 71 14 D2 80 22 01 00 00 01 73 01 00 0F 5F 20 02 20 2F 5F 34 01 01 9F 10 07 06 02 12 03 A0 20 00 9F 26 08 2E C0 60 EC C9 21 1C 8B 9F 27 01 80 9F 36 02 00 2A 9F 6C 02 38 00 9F 6E 04 20 70 00 00 9F 4B 81 80 2F A2 B1 8B E6 BA 8E 29 5C 3B 7D 35 CD B9 5A AC 3B B4 40 B6 E9 09 D5 A7 FF B7 EB 7E 81 39 F2 F9 94 E9 53 09 2E EB 55 F4 12 03 A6 4C 99 BA 93 42 63 EE A5 F0 77 59 E3 43 15 B3 89 73 67 23 60 F4 F4 4D 0C F9 24 86 A9 9C CB C5 47 F4 6E EC 7B 4C 5F D1 BB 82 37 0B D6 CB FB 8C 18 4A 24 16 BC 24 C3 53 55 82 BA 9E 61 82 08 28 34 8A 43 47 8F 10 0A 1E D6 6F 61 3E 50 86 F7 3C AE B9 EE 34 C8 4B 90 00
SendPdol statusCode 00
TLV Node 77
Child Node 82
Child Node 94
Child Node 57
Child Node 5F20
Child Node 5F34
Child Node 9F10
Child Node 9F26
Child Node 9F27
Child Node 9F36
Child Node 9F6C
Child Node 9F6E
Child Node 9F4B
Tag: 77 Length: D7
    Tag: 82 Length: 2
        20 00
    Tag: 94 Length: 4
        10 01 04 00
    Tag: 57 Length: 13
        41 63 69 10 02 56 71 14 D2 80 22 01 00 00 01 73 01 00 0F
    Tag: 5F20 Length: 2
          / 
    Tag: 5F34 Length: 1
        01
    Tag: 9F10 Length: 7
        06 02 12 03 A0 20 00
    Tag: 9F26 Length: 8
        2E C0 60 EC C9 21 1C 8B
    Tag: 9F27 Length: 1
        80
    Tag: 9F36 Length: 2
        00 2A
    Tag: 9F6C Length: 2
        38 00
    Tag: 9F6E Length: 4
        20 70 00 00
    Tag: 9F4B Length: 80
        2F A2 B1 8B E6 BA 8E 29 5C 3B 7D 35 CD B9 5A AC 3B B4 40 B6 E9 09 D5 A7 FF B7 EB 7E 81 39 F2 F9 94 E9 53 09 2E EB 55 F4 12 03 A6 4C 99 BA 93 42 63 EE A5 F0 77 59 E3 43 15 B3 89 73 67 23 60 F4 F4 4D 0C F9 24 86 A9 9C CB C5 47 F4 6E EC 7B 4C 5F D1 BB 82 37 0B D6 CB FB 8C 18 4A 24 16 BC 24 C3 53 55 82 BA 9E 61 82 08 28 34 8A 43 47 8F 10 0A 1E D6 6F 61 3E 50 86 F7 3C AE B9 EE 34 C8 4B
Search for tag 57 (Track 2 Equivalent Data)
tlvNode->getValue length 19
41 63 69 10 02 56 71 14 d2 80 22 01 00 00 01 73 01 00 0f posIndex 0 byte 41 upperByte 04 lowerByte 01
posIndex 1 byte 63 upperByte 06 lowerByte 03
posIndex 2 byte 69 upperByte 06 lowerByte 09
posIndex 3 byte 10 upperByte 01 lowerByte 00
posIndex 4 byte 02 upperByte 00 lowerByte 02
posIndex 5 byte 56 upperByte 05 lowerByte 06
posIndex 6 byte 71 upperByte 07 lowerByte 01
posIndex 7 byte 14 upperByte 01 lowerByte 04
posIndex 8 byte d2 upperByte 0d lowerByte 02
posIndex 9 byte 80 upperByte 08 lowerByte 00
posIndex 10 byte 22 upperByte 02 lowerByte 02
Pan length 16: 4163691002567114
ExpDate length 4: 2802
Search for tag 94 (AFL Application File Locator)
tlvNode->getValue length 4
10 01 04 00 
-------------------------------------------------------------------------
AFL Handling
AFL length 4
Number of AFL entries 1
-------------------------------------------------------------------------
AFL Entry 1
Number of files in AFL entry: 4
-------------------------------------------------------------------------
AFL for SFI 10 file 01
ReadRecord 10 01 04 00
ReadRecord_Le 10 01 04 00
Send length 5
 00 B2 01 14 F8
Recv length 2
 67 00
statusCode 0 backLen 2
Card is asking for Le = 0x00
ReadRecord_Le 10 01 04 00
Send length 5
 00 B2 01 14 00
Recv length 17
 70 0D 8F 01 09 9F 32 03 01 00 01 9F 47 01 03 90 00
*** ReadRecord backLen 17
TLV Node 70
Child Node 8F
Child Node 9F32
Child Node 9F47
Tag: 70 Length: D
    Tag: 8F Length: 1
        09
    Tag: 9F32 Length: 3
        01 00 01
    Tag: 9F47 Length: 1
        03
-------------------------------------------------------------------------
AFL for SFI 10 file 02
ReadRecord 10 02 04 00
ReadRecord_Le 10 02 04 00
Send length 5
 00 B2 02 14 F8
Recv length 2
 67 00
statusCode 0 backLen 2
Card is asking for Le = 0x00
ReadRecord_Le 10 02 04 00
Send length 5
 00 B2 02 14 00
Recv length 255
 0B 06 00 17 00 00 00 63 02 40 3F 0C 2D FC 3F 1C 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 20 03 06 00 17 00 00 00 90 1F FB 3F 60 1F FB 3F 08 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 20 01 06 00 9C 95 0D 80 10 1F FB 3F 00 00 00 00 00 00 00 00 00 00 00 00 90 1F FB 3F 60 1F FB 3F 08 00 00 00 9C 95 0D 80 30 1F FB 3F 00 00 00 00 00 00 00 00 92 99 0D 80 30 1F FB 3F 20 24 FC 3F 0D 36 40 3F 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 74 9A 0D 80 50 1F FB 3F 00 00 00 00 0D 36 40 3F 02 00 00 00 00 FF 00 00 00 00 FF 00 00 00 00 FF 90 9A 0D 80 70 1F FB 3F 20 24 FC 3F 7D 02 40 3F 1C 00 00 00 00 FF 00 00 00 00 FF 00 20 24 FC 3F C4 32 0D 80 90 1F FB 3F 20 24 FC 3F 7D 02 40 3F FC 1E FB 3F 3C 1F FB 3F 0C 2D FC 3F 20 24 FC 3F 90 3A 0D 80 B0 1F FB 3F DC 19 FC 3F
*** ReadRecord backLen 0
Received no valid response, aborting
Error Read Record, skipping
-------------------------------------------------------------------------
AFL for SFI 10 file 03
ReadRecord 10 03 04 00
ReadRecord_Le 10 03 04 00
Send length 5
 00 B2 03 14 F8
Recv length 2
 67 00
statusCode 0 backLen 2
Card is asking for Le = 0x00
ReadRecord_Le 10 03 04 00
Send length 5
 00 B2 03 14 00
Recv length 14
 70 0A 5F 28 02 02 76 9F 07 02 C0 C0 90 00
*** ReadRecord backLen 14
TLV Node 70
Child Node 5F28
Child Node 9F07
Tag: 70 Length: A
    Tag: 5F28 Length: 2
        02 76
    Tag: 9F07 Length: 2
        C0 C0
-------------------------------------------------------------------------
AFL for SFI 10 file 04
ReadRecord 10 04 04 00
ReadRecord_Le 10 04 04 00
Send length 5
 00 B2 04 14 F8
Recv length 2
 67 00
statusCode 0 backLen 2
Card is asking for Le = 0x00
ReadRecord_Le 10 04 04 00
Send length 5
 00 B2 04 14 00
Recv length 211
 70 81 CE 5A 08 41 63 69 10 02 56 71 14 5F 24 03 28 02 29 9F 46 81 B0 33 6D 55 DF 10 4A 56 D4 EC 3E 80 5F 41 91 47 8F D3 21 91 CF 2B 4A 18 BB EB 34 A6 A7 A6 92 63 1F 45 30 1F C1 31 0A 96 08 E7 AC FE 94 8E F8 EB 02 88 DE 6B CD C1 8C 05 EB C6 7F 0D C1 F0 F0 67 49 61 51 C6 DA 89 4E C4 65 1B 95 25 CA 0F 77 20 15 64 C8 6E 09 37 01 C4 2A AD 58 08 1A 03 E9 66 B3 87 9A 4F FA 1A E5 67 2D C5 83 F5 20 81 7E 39 8B DC 78 C4 3F C3 0E 85 3E D6 79 2B 94 96 CB 97 B0 0E 87 A5 50 0E D5 62 E3 5B D9 6C C4 DA 9A 1C B7 62 DC 49 52 45 34 7B 51 CF AE AF 54 7D 7E 7D 59 4A 1D 5C FF 15 90 77 CC DE E9 76 73 4B E3 C2 50 9F 69 07 01 9E D8 A7 A1 00 00 90 00
*** ReadRecord backLen 211
TLV Node 70
Child Node 5A
Child Node 5F24
Child Node 9F46
Child Node 9F69
Tag: 70 Length: CF
    Tag: 5A Length: 8
        41 63 69 10 02 56 71 14
    Tag: 5F24 Length: 3
        28 02 29
    Tag: 9F46 Length: B0
        33 6D 55 DF 10 4A 56 D4 EC 3E 80 5F 41 91 47 8F D3 21 91 CF 2B 4A 18 BB EB 34 A6 A7 A6 92 63 1F 45 30 1F C1 31 0A 96 08 E7 AC FE 94 8E F8 EB 02 88 DE 6B CD C1 8C 05 EB C6 7F 0D C1 F0 F0 67 49 61 51 C6 DA 89 4E C4 65 1B 95 25 CA 0F 77 20 15 64 C8 6E 09 37 01 C4 2A AD 58 08 1A 03 E9 66 B3 87 9A 4F FA 1A E5 67 2D C5 83 F5 20 81 7E 39 8B DC 78 C4 3F C3 0E 85 3E D6 79 2B 94 96 CB 97 B0 0E 87 A5 50 0E D5 62 E3 5B D9 6C C4 DA 9A 1C B7 62 DC 49 52 45 34 7B 51 CF AE AF 54 7D 7E 7D 59 4A 1D 5C FF 15 90 77 CC DE E9 76 73 4B E3 C2 50
    Tag: 9F69 Length: 7
        01 9E D8 A7 A1 00 00
tlvNode->getValue length 8
41 63 69 10 02 56 71 14 PAN found length 8
 41 63 69 10 02 56 71 14
tlvNode->getValue length 3
28 02 29 Expire Date found length 3
 28 02 29
-------------------------------------------------------------------------
PAN found length 8
 41 63 69 10 02 56 71 14
Exp.Date found length 3
 28 02 29
-------------------------------------------------------------------------
 E01 Credit Card Handling END
-------------------------------------------------------------------------         
