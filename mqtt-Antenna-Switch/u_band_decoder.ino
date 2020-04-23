
void setBand(uint16_t rNum, char* txFreq, uint16_t fLen) {
  // set band fields based on txFreq with resolution to 10 Hz

  /* Bandpasser II - BCD code band control on Band Data pins A, B, C, D
  Band    Code  Hex
  160m    0001  0x1
  80m     0010  0x2
  40m     0011  0x3
  30m     0100  0x4
  20m     0101  0x5
  17m     0110  0x6
  15m     0111  0x7
  12m     1000  0x8
  10m     1001  0x9
  6m      1010  0xA
  Bypass  00000 0x0
  */

  // calculate 100 Khz range of txFreq

  // String freq;
  // String hundredkhz;
  char hundredkhz[11] = {'0','0','0','0','0'};
  String str;

  if (fLen > 5) {
    for (int i = 0; i < (fLen - 4); i++) {
      hundredkhz[i] = txFreq[i];
    }
  }
  else {
    fLen = 5;
  }
  hundredkhz[fLen - 4] = '\0';

  //Serial.print(hundredkhz);
  //Serial.print(" == ");

  // int val = Integer.parseInt(hundredkhz);
  int val = atoi(hundredkhz);
  //Serial.println(val);

  if (18 <= val && val <= 19) {
    (str = "160M").toCharArray(radioTable[rNum].bandText, str.length());
    radioTable[rNum].bandNum = 1;
    radioTable[rNum].bandCode = 0x01;
  }
  else if (35 <= val && val <= 39) {
    (str = "80M").toCharArray(radioTable[rNum].bandText, str.length());
    radioTable[rNum].bandNum = 2;
    radioTable[rNum].bandCode = 0x02;
  }
  else if (53 <= val && val <= 54) {
    (str = "60M").toCharArray(radioTable[rNum].bandText, str.length());
    radioTable[rNum].bandNum = 0;
    radioTable[rNum].bandCode = 0x00;
  }
  else if (70 <= val && val <= 72) {
    (str = "40M").toCharArray(radioTable[rNum].bandText, str.length());
    radioTable[rNum].bandNum = 3;
    radioTable[rNum].bandCode = 0x03;
  }
  else if (100 <= val && val <= 101) {
    (str = "30M").toCharArray(radioTable[rNum].bandText, str.length());
    radioTable[rNum].bandNum = 4;
    radioTable[rNum].bandCode = 0x04;
  }
  else if (140 <= val && val <= 143) {
    (str = "20M").toCharArray(radioTable[rNum].bandText, str.length());
    radioTable[rNum].bandNum = 5;
    radioTable[rNum].bandCode = 0x05;
    }
  else if (180 <=val && val <= 181) {
    (str = "17M").toCharArray(radioTable[rNum].bandText, str.length());
    radioTable[rNum].bandNum = 6;
    radioTable[rNum].bandCode = 0x06;
  }
  else if (210 <= val && val <= 214) {
    (str = "15M").toCharArray(radioTable[rNum].bandText, str.length());
    radioTable[rNum].bandNum = 7;
    radioTable[rNum].bandCode = 0x07;
  }
  else if (248 <= val && val <= 249) {
    (str = "12M").toCharArray(radioTable[rNum].bandText, str.length());
    radioTable[rNum].bandNum = 8;
    radioTable[rNum].bandCode = 0x08;
  }
  else if (280 <= val && val <= 297) {
    (str = "10M").toCharArray(radioTable[rNum].bandText, str.length());
    radioTable[rNum].bandNum = 9;
    radioTable[rNum].bandCode = 0x09;
  }
  else if (500 <= val && val <= 549) {
    (str = "6M").toCharArray(radioTable[rNum].bandText, str.length());
    radioTable[rNum].bandNum = 10;
    radioTable[rNum].bandCode = 0x0A;
  }
  else if (1440 <= val && val <= 1479) {
    (str = "2M").toCharArray(radioTable[rNum].bandText, str.length());
    radioTable[rNum].bandNum = 0;
    radioTable[rNum].bandCode = 0x00;
  }
  else if (4300 <= val && val <= 4499) {
    (str = "70cm").toCharArray(radioTable[rNum].bandText, str.length());
    radioTable[rNum].bandNum = 0;
    radioTable[rNum].bandCode = 0x00;
  }
  else if (9020 <= val && val <= 9279) {
    (str = "33cm").toCharArray(radioTable[rNum].bandText, str.length());
    radioTable[rNum].bandNum = 0;
    radioTable[rNum].bandCode = 0x00;
  }
  else if (1240 <= val && val <= 1299) {
    (str = "23cm").toCharArray(radioTable[rNum].bandText, str.length());
    radioTable[rNum].bandNum = 0;
    radioTable[rNum].bandCode = 0x00;
  }
  else {
    (str = "GEN").toCharArray(radioTable[rNum].bandText, str.length());
    radioTable[rNum].bandNum = 0;
    radioTable[rNum].bandCode = 0x00;
  }
return;
}
