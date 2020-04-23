/*----------------------------------------------------------------------------
/*  This file contains the functions that replicate the telnet command
/*  funcitons of the original Antenna Switch controller.
/*  Creating this capability to be backward compatible with the current
/*  Python based GUI antenna controller software to allow deployment before
/*  refactoring the Python GUI controller and Java based Radion Antenna
/*  Tracker software.
/*--------------------------------------------------------------------------*/

/* These functions require the following to be declared in the main file.
//
//  unsigned int telnetPort = 23;
//  WiFiServer server(telnetPort);
//
//  // Define variables and functions
// char replyBuffer[1024];
// char buffer[1024];         // buffer for status message PString conversions
// String s = "" ;           // usde for client reply formation
// unsigned int l;           // temp variable for command buffer to string conversion
// char r[128];               // buffer for command conversion to char string type
// boolean wifiConnected = false;
// boolean connectWifi();
// String command;           // received command string
// String req = "";          // received command string
// PString msg_Status(buffer, sizeof(buffer));
// String msg_Stat = "";
//
// unsigned long rt_total = 0;
// int num_req = 0;
// unsigned long rt_max = 0;
// unsigned long rt_min = 0;
// unsigned long rt_avg = 0;
// unsigned long rt_last = 0;
// unsigned long t0 = 0; // initial time
// unsigned long t1 = 0; // delta time 1
// unsigned long t2 = 0; // delta time 2
// unsigned long t3 = 0; // delta time 3
// unsigned long t4 = 0; // delta time 4
// unsigned long t5 = 0; // delta time 5
// unsigned long t6 = 0; // delta time 6
// unsigned long t7 = 0; // delta time 7

// const int antS1 = 12;
// const int antS2 = 16;
// const int antS3 = 4;
// const int antS4 = 10;
// const int antS5 = 15;
// const int antS6 = 5;
// const int antS7 = 13;
// const int antS8 = 14;
//
// const int antpin[] = {12, 16, 4, 10, 15, 5, 13, 14};
//

/* The following is required in the setup() function:
// // start server
// server.begin();
// server.setNoDelay(true);

*/

/*----------------------------------------------------------------------------
/*  This function contains the code from the main loop() of
/*  My-WiFi-UDP_Telnet-LED_control-v17(v18)
/*--------------------------------------------------------------------------*/
void telnetLoop() {
  uint8_t i;
  uint8_t n;
  boolean valid = false;
  char inChar = '\0';
  char buf[128];
  PString logMsg(buf, sizeof(buf));


  // check for new clients to the server
  checkNewClient();

  // now check clients for data available and read it
  for(i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (checkClientInput(i) > 0) {
      // get input from client[i]
      // if a terminated line was received from client
      // return is positive - input line received - is this a command?

      t0 = micros();  // intialize time when command is received
      valid = parseTelnet(i); // go parse the completed input

      if (valid) {  // valid command - send a response to all the clients
        // publish an MQTT log message with received command
        logMsg.begin();
        logMsg.print("Telnet Command Received: ");
        logMsg.print(InBuf[i].inputLine);
        publishLog(logMsg);

        // Assemble the response string
        strcpy(s, "\r\n+");
        strcat(s, InBuf[i].inputLine);
        strcat(s, ">>");
        msg_Stat += s;

        // send response to all connected clients
        for (n = 0; n < MAX_SRV_CLIENTS; n++) {
          if (serverClients[n] != NULL) {
            if (serverClients[n]->connected()) {
              serverClients[n]->write(msg_Stat.c_str());
            }
          }
        } // end of for loop
        // clear the buffer after sending the response to the clients
        InBuf[i].inputLine[0] = '\0';  // initialize InBuf for next command
        InBuf[i].charCount = 0; // reset input charact count
      }

      else if (!valid) {
        // invalid command was received - send invalid response to all clients
        #ifdef DEBUG_TELNET
        Serial.print("Invalid request -->");
        Serial.println(InBuf[i].inputLine);
        Serial.print("\r\nSW_Ready>>");
        #endif

        // publish an MQTT log entry if an error
        logMsg.begin();
        logMsg.print("Invalid Telnet Command Received: ");
        logMsg.print(InBuf[i].inputLine);
        publishLog(logMsg);

        // send response to all connected clients
        for (n = 0; n < MAX_SRV_CLIENTS; n++) {
          if (serverClients[n] != NULL) {
            if (serverClients[n]->connected()) {
              serverClients[n]->write("\r\nInvalid request -->");
              serverClients[n]->write(InBuf[i].inputLine, strlen(InBuf[i].inputLine));
              serverClients[n]->write("\r\nSW_Ready>>");
            }
          }
        }
        // clear the buffer after sending response to all clients
        InBuf[i].inputLine[0] = '\0';  // initialize InBuf for next command
        InBuf[i].charCount = 0; // reset input charact count
      }
    // no input to take action on for this client - continue loop
    }
  } // end for lookup

} // end telnetLoop

/*-----------------------------------------------------------------------------
/*    Function to check for new client connect requests
/*      - if the server available is true
/*        - then add the client to serverClients
/*---------------------------------------------------------------------------*/
void checkNewClient() {
  char buf[128];
  PString logMsg(buf, sizeof(buf));

  WiFiClient newClient = server.available();
  if (newClient) {
    #ifdef DEBUG_TELNET
    Serial.print("\r\nNew telnet client connecting...");
    #endif
    // add a new server connection
    for(uint8_t i = 0; i < MAX_SRV_CLIENTS; i++) {
      // if there is free client slot use it
      if (serverClients[i] == NULL) {

        // now fill the available slot with the new client
        serverClients[i] = new WiFiClient(newClient);
        serverClients[i]->write("SW_Ready>>");
        InBuf[i].charCount = 0; // initialize input buffer char counter
        InBuf[i].inputLine[0] = '\0';  // initialize InBuf for next command

        // publish an MQTT log message with received command
        logMsg.begin();
        logMsg.print("New Telnet client: ");
        logMsg.print(i);
        publishLog(logMsg);

        #ifdef DEBUG_TELNET
        Serial.print("New Telnet client: ");
        Serial.println(i);
        #endif

        break;  // found an open slot - break out of for loop
      }  // end if NULL

      // if this client is disconnected free the slot
      if (!serverClients[i]->connected()) {
        serverClients[i]->stop();
        delete serverClients[i];
        serverClients[i] = NULL;

        #ifdef DEBUG_TELNET
        Serial.print("Disconnect stale client: ");
        Serial.println(i);
        #endif

        // now fill the available slot with the new client
        serverClients[i] = new WiFiClient(newClient);
        serverClients[i]->write("SW_Ready>>");
        serverClients[i]->flush(); // be sure the buffers are clear
        InBuf[i].charCount = 0; // initialize input buffer char counter
        InBuf[i].inputLine[0] = '\0';  // initialize InBuf for next command

        // publish an MQTT log message with received command
        logMsg.begin();
        logMsg.print("Disconnect & New Telnet client: ");
        logMsg.print(i);
        publishLog(logMsg);

        #ifdef DEBUG_TELNET
        Serial.print("Disconnect & New Telnet client: ");
        Serial.println(i);
        #endif

        break;  // found an open slot - break out of for loop
      } // end not connected if

      if (i == MAX_SRV_CLIENTS) {

        #ifdef DEBUG_TELNET
        Serial.print("serverClient[i] = ");
        Serial.println(i);
        #endif

        logMsg.begin();
        logMsg.print("Refused connection - Max Telnet clients connected: ");
        logMsg.print(i);
        publishLog(logMsg);

        #ifdef DEBUG_TELNET
        Serial.print("Refused connection - Max Telnet clients connected: ");
        Serial.println(i);
        #endif

        newClient.flush();
        newClient.stop();
      }
    }  // end add new client for loop
  } // end if (newClient)
}


/*----------------------------------------------------------------------------
/*  This function checks a telnet client for input
/*      - reads available input and stores in global client buffers[i]
/*      - returns an integer to indicate state of inputLine
/*          0 = means no data in buffer[i]
/*          -int = # of chars in buffer[i] - no terminator received
/*          +int = terminator received - # of chars in buffer[i]
/*---------------------------------------------------------------------------*/
int checkClientInput(uint8_t i) {
  char inChar;

  //now check client[i] for data to read
  if (serverClients[i] != NULL) {  // check if this client initialized
    if (serverClients[i]->connected()) {  // check if connected
      //get data from the telnet client and push it to the UART
      while(serverClients[i]->available()) {
        // command = serverClients[i]->readStringUntil('\n');
        inChar = serverClients[i]->read();
        if (validInput(inChar)) {
          // append it to input buffer if valid
          Serial.print(inChar);
          if (InBuf[i].charCount < MAX_LINE_LENGTH-2) {
            InBuf[i].inputLine[InBuf[i].charCount] = inChar;
            InBuf[i].charCount += 1;  // increment the character count
          }
          if (inChar == '\n') { // if end of line add string terminator
            InBuf[i].inputLine[InBuf[i].charCount] = '\0';
            // complement the charCount value and return
            return InBuf[i].charCount;
          } // end if (terminator character)
        } // end if (validInput)
      }  // end while->available()
    } // end if ->connected()
  } // end if != NULL
  return -InBuf[i].charCount;
}
