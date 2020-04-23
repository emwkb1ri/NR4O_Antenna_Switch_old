/*------------------------------------------------------------------------
/*          Function to parse Cmd messages and take action
/*----------------------------------------------------------------------*/
void parseCmd() {
  char s[11] = "";
  char buf[80];
  PString error(buf, sizeof(buf));

  newMsg = false;  // clear the newMsg flag

  // is this an 's' command?
  if (msg[0] == 's') {
    // Is this a reset command?
    if (strcmp(msg, "sMQTT") == 0) {
    // if sMQTT - reset the MQTT client connection
    Serial.println("Resetting MQTT client connection - not implemented");
    goto end;
    }
    // if not then check for 'st:'
    else {
      // if msg[0-2] == "st:"
      for (unsigned int i = 0; i < 3; i++) {
        s[i] = msg[i];
      }
      s[3] = '\0';
      if (strcmp(s, "st:") == 0) {
        // it's the set interval command
        // now convert characters after st: to integer
        // need to verify each character is a digit
        unsigned int i = 3;
        unsigned int m = 0;   // temp integer
        while (msg[i] != '\0') {
          // first verify that they are all numbers and convert to integer
          if (msg[i] >= '0' && msg[i] <= '9') {
            m = m * 10 + (msg[i] - '0'); // convert to an integer
            i++;
          }
          else {
            Serial.print("ERROR 2: Bad value in 'st:' CMD: ");
            Serial.println(msg);
            error.begin();
            error.print("ERROR 2: Bad value in 'st:' CMD: ");
            error.print(msg);
            publishLog(error);
            goto end;
          }
        }
        statInterval = m * 1000UL;  // now update interval as m milliseconds
        goto end;
      }  // end of "st:"
    }
  }
  // check if this is '#' command
  if (msg[0] == '#') {
      // check message for a command

      if (strcmp(msg, "#h") == 0) {
        // halt auto status updates
        statusFlag = false;
      }
      else if (strcmp(msg, "#H") == 0) {
        // halt auto status updates
        statusFlag = false;
      }
      else if (strcmp(msg, "#s") == 0) {
        // publish status now and leave statusFlag unchanged
        pubStatus();
      }
      else if (strcmp(msg, "#S") == 0) {
        // Start auto updates
        statusFlag = true;
      }
      else if (strcmp(msg, "#u") == 0) {
        // Stop UDP mode
        udp_mode = false;
      }
      else if (strcmp(msg, "#U") == 0) {
        // Start UDP mode
        udp_mode = true;
      }

      #ifdef LINKNODE_R4
      else if (strcmp(msg,"#0") == 0) {
        // clear all relays/LEDs
        clearRelays();
        resp_time();  // calculate response time
      }
      else if (strcmp(msg, "#1") == 0) {
        // clear all relays/LEDs
        clearRelays();
        digitalWrite(aPin[0], HIGH);
        resp_time();  // calculate response time
      }
      else if (strcmp(msg, "#2") == 0) {
        // clear all relays/LEDs
        clearRelays();
        digitalWrite(aPin[1], HIGH);
        resp_time();  // calculate response time
      }
      else if (strcmp(msg, "#3") == 0) {
        // clear all relays/LEDs
        clearRelays();
        digitalWrite(aPin[2], HIGH);
        resp_time();  // calculate response time
      }
      else if (strcmp(msg, "#4") == 0) {
        // clear all relays/LEDs
        clearRelays();
        digitalWrite(aPin[3], HIGH);
        resp_time();  // calculate response time
      }
      #else
      else if (strcmp(msg,"#0") == 0) {
        // clear all relays/LEDs
        clearRelays();
        resp_time();  // calculate response time
      }
      else if (strcmp(msg, "#1") == 0) {
        // clear all relays/LEDs
        clearRelays();
        digitalWrite(aPin[0], HIGH);
        resp_time();  // calculate response time
      }
      else if (strcmp(msg, "#2") == 0) {
        // clear all relays/LEDs
        clearRelays();
        digitalWrite(aPin[1], HIGH);
        resp_time();  // calculate response time
      }
      else if (strcmp(msg, "#3") == 0) {
        // clear all relays/LEDs
        clearRelays();
        digitalWrite(aPin[2], HIGH);
        resp_time();  // calculate response time
      }
      else if (strcmp(msg, "#4") == 0) {
        // clear all relays/LEDs
        clearRelays();
        digitalWrite(aPin[3], HIGH);
        resp_time();  // calculate response time
      }
      else if (strcmp(msg, "#5") == 0) {
        // clear all relays/LEDs
        clearRelays();
        digitalWrite(aPin[4], HIGH);
        resp_time();  // calculate response time
      }
      else if (strcmp(msg, "#6") == 0) {
        // clear all relays/LEDs
        clearRelays();
        digitalWrite(aPin[5], HIGH);
        resp_time();  // calculate response time
      }
      else if (strcmp(msg, "#7") == 0) {
        // clear all relays/LEDs
        clearRelays();
        digitalWrite(aPin[6], HIGH);
        resp_time();  // calculate response time
      }
      else if (strcmp(msg, "#8") == 0) {
        // clear all relays/LEDs
        clearRelays();
        digitalWrite(aPin[7], HIGH);
        resp_time();  // calculate response time
      }
      #endif

      else {
        // it must be an invalid command if it got here
        Serial.print("ERROR 1: Invalid CMD: ");
        Serial.println(msg);
        error.begin();
        error.print("ERROR 1: Invalid CMD: ");
        error.print(msg);
        publishLog(error);
      }
    }
    else {

      // it must be an invalid command if it got here
      Serial.print("ERROR 1: Invalid CMD: ");
      Serial.println(msg);
      error.begin();
      error.print("ERROR 1: Invalid CMD: ");
      error.print(msg);
      publishLog(error);
    }
  end:
  return;
}  // end parseCmd()
