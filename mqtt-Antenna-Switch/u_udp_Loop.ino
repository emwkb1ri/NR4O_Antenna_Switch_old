/*----------------------------------------------------------------------------
/* This file contains the udp_mode data tracking functions
/*---------------------------------------------------------------------------*/

void udpLoop() {

  // if there's data available, read a packet
  int packetSize = OTRSP_UDP.parsePacket();
  // Serial.println(packetSize);
  while (packetSize) {

    unsigned int t = millis(); // start timer
    int i = 0;
    char c;

    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = OTRSP_UDP.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(OTRSP_UDP.remotePort());

    /* read the packet into packetBufffer
    int len = OTRSP_UDP.read(udpBuffer, 1500);
    udpBuffer[len] = '\0'; // add null to end of data packet
    if (len == -1) {
      Serial.println("UDP ERROR: No buffer is available");
    }
    */
    xml.reset();
    xmlEnd = false;

    while((c = OTRSP_UDP.read()) != 0 && !xmlEnd) {
      xml.processChar(c);
      replyBuffer[i] = c;  // save for debug printing
      i++;
    }
    // print the data received to the console
    // Serial.print("Contents: ");
    // Serial.println(packetSize);
    // Serial.println(replyBuffer);

    if (radioTable[0].newData) {

      otrspTimer(1);  // reset the otrspTimeOut timer
      udpSetPins(0); // call the set pin function

      // done with new data
      radioTable[0].newData = false;
    }

    if (radioTable[1].newData) {

      otrspTimer(1);  // reset the otrspTimeOut timer
      udpSetPins(1);  // go set the output pin values

      // done with new data
      radioTable[1].newData = false;
    }
    /*  Listen only for now
    // send a reply, to the IP address and port that sent us the packet we received
    OTRSP_UDP.beginPacket(OTRSP_UDP.remoteIP(), OTRSP_UDP.remotePort());
    OTRSP_UDP.write(replyBuffer);
    OTRSP_UDP.endPacket();
    */
  packetSize = OTRSP_UDP.parsePacket();  // check for additional packets

  }
  otrspTimer(0);  // check time out timer
  // check for a udp time out and call set pins function
  if (otrspTimerExpired) {
    udpSetPins(0);  // reset decoder mode pins associated with Radio 1
    udpSetPins(1);  // reset decoder mode pins associated with Radio 2
    otrspTimer(1); // reset the time out timer
  }
// UDP code insert ends here
}   // end udpLoop function


/*-----------------------------------------------------------------------
/   Function to set the pins on the controller after valid udp packet
/-----------------------------------------------------------------------*/

void udpSetPins(int x) {

  byte code = 0;

  // only set pins on the switches if this is not a otrspTimeOut
  if (otrspTimerExpired == 0) {
    if (x = 0) {
      Serial.print("R1 - newData = ");
    }

    if (x = 1) {
      Serial.print("R2 - newData = ");
    }

    // process radio info here

    Serial.print(radioTable[x].txFreq);
    Serial.print(" - ");
    Serial.print(radioTable[x].antNum);
    Serial.print(" - ");
    Serial.print(radioTable[x].bandText);
    Serial.print(" - ");
    Serial.print(radioTable[x].bandNum);
    Serial.print(" - ");
    Serial.println(radioTable[x].bandCode, HEX);
    Serial.println();

  }

  if (switch_1 && (otrspTimerExpired != 1)) {
    // set antenna to antNum
    // clear all relays/LEDs
    clearRelays();
    if (radioTable[0].antNum != 0) {
      // Set the ant relay control pin if antNum is not 0
      digitalWrite(aPin[radioTable[0].antNum - 1], HIGH);
    }
  }
  else if (switch_2 && (otrspTimerExpired != 1)) {
    // set antenna to antNum
    // clear all relays/LEDs
    clearRelays();
    if (radioTable[1].antNum != 0) {
      // Set the ant relay control pin if antNum is not 0
      digitalWrite(aPin[radioTable[1].antNum - 1], HIGH);
    }
  }
  else if (decoder) {
    // set the band decoder pins for radio 1 (aPin[0-3])
    //if (false) { // disable the decoder timeout
    if (otrspTimerExpired) {
        code = 0; // set the band decoder value to bypass
        radioTable[0].bandCode = 0;
    }
    else {
      code = radioTable[0].bandCode;
    }
    for (int i = 0; i < 4; i++) {
      // if the antCode bit is 1 set pin i HIGH otherwise LOW
      if (code & 0x1) {
        digitalWrite(aPin[i], HIGH);
      }
      else{
        digitalWrite(aPin[i], LOW);
      }
      code = code >> 1;
    }

    // set the band decoder pins for radio 2 (aPin[4-7])
    //if (false) {  // disable the decoder timeout
    if (otrspTimerExpired) {
        code = 0; // set the band decoder value to bypass
        radioTable[1].bandCode = 0;
    }
    else {
      code = radioTable[1].bandCode;
    }
    for (int i = 4; i < 8; i++) {
      // if the antCode bit is 1 set pin i HIGH otherwise LOW
      if (code & 0x1) {
        digitalWrite(aPin[i], HIGH);
      }
      else{
        digitalWrite(aPin[i], LOW);
      }
      code = code >> 1;
    }
  }
}


/*-----------------------------------------------------------------------
/   Function to check for stopped otrsp udp packets
/-----------------------------------------------------------------------*/

void otrspTimer(unsigned int param) {

  // if param = 0 just check for timeout; return 1 if timeout reached

  if (param == 1 ) {
    otrspMillis = millis();  // reset to current time
    otrspTimerExpired = 0;
  }
  else if (millis() - otrspMillis > otrspTimeOut)  {
    // if the time out is reached return true
    otrspTimerExpired = 1;
    otrspTimerStatus = 1;
  }
  return;
}
