/*--------------------------------------------------------------------------
/*Function to start a UDP client/server for NTP messages on port 123
/*-------------------------------------------------------------------------*/
void startNTP_UDP() {
  Serial.print("Starting NTP UDP client/server on ");
  NTP_UDP.begin(123); // Start listening for UDP messages on port 123
  Serial.print("port: ");
  Serial.println(NTP_UDP.localPort());
  Serial.println();
}

/*----------------------------------------------------------------------------
/*            Function to get current time from NTP servier
/*--------------------------------------------------------------------------*/
void getNTPtime() {
  unsigned long currentMillis = millis();
  uint32_t actualTime = 0;

  if (currentMillis - prevNTP > intervalNTP || prevNTP == 0) { // If a minute has passed since last NTP request
    prevNTP = currentMillis;
    Serial.println("\r\nSending NTP request ...");
    sendNTPpacket(timeServerIP); // Send an NTP request
  }
  uint32_t time = getTime2(); // Check if an NTP response has arrived and get the (UNIX) time
  if (time) { // If a new timestamp has been received
    flagNTP = true;  // initial NTP time received so setup can proceed
    timeUNIX = time;
    lastNTPResponse = currentMillis;
    actualTime = timeUNIX + (currentMillis - lastNTPResponse)/1000;
    Serial.printf("UTC time: %d:%d:%d \r\n", getHours(actualTime), getMinutes(actualTime), getSeconds(actualTime));
  }
  else if ((currentMillis - lastNTPResponse) > (3600000UL * 24UL)) {
    Serial.println("More than 24 hours since last NTP response. Rebooting.");
    Serial.flush();
    ESP.reset();
  }
  actualTime = timeUNIX + (currentMillis - lastNTPResponse)/1000;
  if (actualTime != prevActualTime && timeUNIX != 0) { // If a second passed since last print
    prevActualTime = actualTime;
    // Serial.printf("\nUTC time: %d:%d:%d ", getHours(actualTime), getMinutes(actualTime), getSeconds(actualTime));
  }
}

/*----------------------------------------------------------------------------
/* Helper functions for NTP time server functions
/*---------------------------------------------------------------------------*/

uint32_t getTime2() {
  if (NTP_UDP.parsePacket() == 0) { // If there's no response (yet)
    return 0;
  }
  NTP_UDP.read(NTPBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  // Combine the 4 timestamp bytes into one 32-bit number
  uint32_t NTPTime = (NTPBuffer[40] << 24) | (NTPBuffer[41] << 16) | (NTPBuffer[42] << 8) | NTPBuffer[43];
  // Convert NTP time to a UNIX timestamp:
  // Unix time starts on Jan 1 1970. That's 2208988800 seconds in NTP time:
  const uint32_t seventyYears = 2208988800UL;
  // subtract seventy years:
  uint32_t UNIXTime = NTPTime - seventyYears;
  return UNIXTime;
}

// send a packet to the NTP server
void sendNTPpacket(IPAddress& address) {
  memset(NTPBuffer, 0, NTP_PACKET_SIZE); // set all bytes in the buffer to 0
  // Initialize values needed to form NTP request
  NTPBuffer[0] = 0b11100011; // LI, Version, Mode
  // send a packet requesting a timestamp:
  NTP_UDP.beginPacket(address, 123); // NTP requests are to port 123
  NTP_UDP.write(NTPBuffer, NTP_PACKET_SIZE);
  NTP_UDP.endPacket();
}

inline int getSeconds(uint32_t UNIXTime) {
  return UNIXTime % 60;
}

inline int getMinutes(uint32_t UNIXTime) {
  return UNIXTime / 60 % 60;
}

inline int getHours(uint32_t UNIXTime) {
  return UNIXTime / 3600 % 24;
}
