/*----------------------------------------------------------------------------
/*     Function to parse the command received from a telnet client
/*---------------------------------------------------------------------------*/

bool parseTelnet(uint8_t i) {
  boolean valid = false;
  String s = "";

  msg_Stat = "";  // clear the msg_Stat string

  // get the command bytes from the client read buffer
  if (InBuf[i].inputLine[0] == 'A' || InBuf[i].inputLine[0] == 'a' ||
            InBuf[i].inputLine[0] == 'B' || InBuf[i].inputLine[0] == 'b') {
    if (InBuf[i].inputLine[1] == 'S' || InBuf[i].inputLine[1] == 's') {
      printSwitchData();
      msg_Stat = msg_Status;  // append PString msg_Status to String msg_Stat
      valid = true;
    }
    else if (InBuf[i].inputLine[1] == '0') {
      Serial.println("\r\nAntenna A0 - All OFF... 00000000");
      clearRelays();
      resp_time(); // calculate response time of command processing
      valid = true;
    }
    else if (InBuf[i].inputLine[1] == '1') {
      Serial.println("\r\nAntenna A1 - ON... 10000000");
      clearRelays();
      digitalWrite(aPin[0], HIGH);
      resp_time(); // calculate response time of command processing
      valid = true;
    }
    else if (InBuf[i].inputLine[1] == '2') {
      Serial.println("\r\nAntenna A2 - ON... 01000000");
      clearRelays();
      digitalWrite(aPin[1], HIGH);
      resp_time(); // calculate response time of command processing
      valid = true;
    }
    else if (InBuf[i].inputLine[1] == '3') {
      Serial.println("\r\nAntenna A3 - ON... 00100000");
      clearRelays();
      digitalWrite(aPin[2], HIGH);
      resp_time(); // calculate response time of command processing
      valid = true;
    }
    else if (InBuf[i].inputLine[1] == '4') {
      Serial.println("\r\nAntenna A4 - ON... 00010000");
      clearRelays();
      digitalWrite(aPin[3], HIGH);
      resp_time(); // calculate response time of command processing
      valid = true;
    }

    #ifndef LINKNODE_R4
    else if (InBuf[i].inputLine[1] == '5') {
      Serial.println("\r\nAntenna A5 - ON... 00001000");
      clearRelays();
      digitalWrite(aPin[4], HIGH);
      resp_time(); // calculate response time of command processing
      valid = true;
    }
    else if (InBuf[i].inputLine[1] == '6') {
      Serial.println("\r\nAntenna A6 - ON... 00000100");
      clearRelays();
      digitalWrite(aPin[5], HIGH);
      resp_time(); // calculate response time of command processing
      valid = true;
    }
    else if (InBuf[i].inputLine[1] == '7') {
      Serial.println("\r\nAntenna A7 - ON... 00000010");
      clearRelays();
      digitalWrite(aPin[6], HIGH);
      resp_time(); // calculate response time of command processing
      valid = true;
    }
    else if (InBuf[i].inputLine[1] == '8') {
      Serial.println("\r\nAntenna A8 - ON... 00000001");
      clearRelays();
      digitalWrite(aPin[7], HIGH);
      resp_time(); // calculate response time of command processing
      valid = true;
    }
    #endif
  } // end of valid commands

  return valid;
}
