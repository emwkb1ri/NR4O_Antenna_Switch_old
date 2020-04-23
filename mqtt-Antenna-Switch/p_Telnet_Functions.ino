/*----------------------------------------------------------------------------
/*  This file contains the functions that replicate the telnet command
/*  funcitons of the original Antenna Switch controller.
/*  Creating this capability to be backward compatible with the current
/*  Python based GUI antenna controller software to allow deployment before
/*  refactoring the Python GUI controller and Java based Radion Antenna
/*  Tracker software.
/*--------------------------------------------------------------------------*/


void printSwitchData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();

  // initialize message status string to be returned on status command
  msg_Status.begin();

  // Append the SSID to the status string
  msg_Status.print("SSID: ");
  msg_Status.println(WiFi.SSID());

  // Append IP info to status string
  msg_Status.print("IP Address: ");
  msg_Status.print(ip);
  msg_Status.print("\r\n");

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);

/*  Serial.print("Module MAC address: ");
  Serial.print(mac[5],HEX);
  Serial.print(":");
  Serial.print(mac[4],HEX);
  Serial.print(":");
  Serial.print(mac[3],HEX);
  Serial.print(":");
  Serial.print(mac[2],HEX);
  Serial.print(":");
  Serial.print(mac[1],HEX);
  Serial.print(":");
  Serial.println(mac[0],HEX);
*/

  // Append MAC address to the status string
  msg_Status.print("Module MAC address: ");
  msg_Status.print(mac[0],HEX);
  msg_Status.print(":");
  msg_Status.print(mac[1],HEX);
  msg_Status.print(":");
  msg_Status.print(mac[2],HEX);
  msg_Status.print(":");
  msg_Status.print(mac[3],HEX);
  msg_Status.print(":");
  msg_Status.print(mac[4],HEX);
  msg_Status.print(":");
  msg_Status.println(mac[5],HEX);

  // print the received signal strength:
  long rssi = WiFi.RSSI();

  // Append signal strength and firmware version number to status string
  msg_Status.print("Signal strength (RSSI): ");
  msg_Status.print(rssi);
  msg_Status.println(" dBm");
  msg_Status.print("Module Version: ");
  msg_Status.println(ver_num);

  // Append response time calculations
  msg_Status.print("Requests: ");
  msg_Status.print(num_req);
  msg_Status.print("  Last: ");
  msg_Status.print(rt_last);
  msg_Status.println("uSec");
  msg_Status.print("Avg resp time: ");
  msg_Status.print(rt_avg);
  msg_Status.print("uSec");
  msg_Status.print("  Min: ");
  msg_Status.print(rt_min);
  msg_Status.print("  Max: ");
  msg_Status.println(rt_max);

  // get pin state

  byte pin_state = 0;
  unsigned int pin_val[RELAYS];

  // append pin state to status message string
  msg_Status.print("Antenna pin state: ");
//  Serial.print("Antenna pin state: ");
  for (int i = 0; i < RELAYS; i++) {
    pin_val[i] = 0;
    pin_val[i] = digitalRead(aPin[i]);
//    Serial.print(pin_val[i]);
    msg_Status.print(pin_val[i]);
  }
  // add a cr/newline at the end of the pin state status line
  msg_Status.print("\r\n");

  // Append a separator line
  msg_Status.println("###########################");

  // Print assembled WiFi status message on serial console for debugging
  Serial.println("");
  Serial.print(msg_Status);
}

void getCurrentPins() {
  // get pin state

  byte pin_state = 0;
  unsigned int pin_val[RELAYS];
  msg_Status.begin(); // re-initialize
  msg_Status.print("Antenna pin state: ");

  // append pin state to status message string
  for (int i = 0; i < RELAYS; i++) {
    pin_val[i] = 0;
    pin_val[i] = digitalRead(aPin[i]);
    msg_Status.print(pin_val[i]);
  }

  // append a cr/newline to the end of the message
  msg_Status.print("\r\n");

  // print assembled status message to console for debugging
  Serial.println(msg_Status);
}

/*---------------------------------------------------------------------------
/*  Function returns true if the input character is any one of:
/*  '0' - '9', 'A' = 'Z', 'a' - 'z', '#', ':', ';', '\r', '\n'
/*--------------------------------------------------------------------------*/
bool validInput(char input) {

  if (input >= '0' && input <= '9') return true;
  if (input >= 'A' && input <= 'Z') return true;
  if (input >= 'a' && input <= 'z') return true;
  if (input == '\r' || input == '\n') return true;
  if (input == '#' || input == ':' || input == ';') return true;
  return false;
}
