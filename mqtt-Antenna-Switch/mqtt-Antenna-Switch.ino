/*
 WiFi/Telnet antenna relay controller using LinkSprite LinkNode R8 ESP8266 controller

  Copyright (c) 2018, 2019 Eric Wagner. All rights reserved.
  This file is part of the ESP8266WiFi library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/****************************************************************************
  NR4O Antenna Switch Controller - based on NodeMCU-WiFiMQTTCallback sketch

  This sketch uses an MQTT broker to control either a LinkNode R4 or R8
  relay board.  The relay board in turn controls either a 4-port antenna
  switch or an 8-port antenna switch.

  **Use the WeMos D1(Retired) board in the Arduino IDE ESP8266/NodeMCU board
  manager.

  Antenna Controller Mode - GPIO pins - {12, 16, 4, 10, 15, 5, 13, 14}
  LinkNode R4 only uses - {12, 16, 4, 10} (Not tested)

  **Requires the PString library for some string manipulation.  It can be found
  at http://arduiniana.org/libraries/pstring/

  **The ESP8266WiFi library is required for these LinkNode boards to manage the
  WiFi module.

  **The ArduinoMqttClient library is used for the MQTT protocol.

  **The MQTT client expects to connect to an MQTT broker.  I am currently
    using a mosquitto MQTT broker running on a Raspberry PI 3+.  The MQTT
    control path can be used at the same time as the OTRSP UDP and telnet
    control paths.  Just be aware that the last control command to the
    controllers will set the antenna or decoder switch state.
    
  **The current MQTT Topic structure is as follows:
    ESP_xxxx = NodeMCU hostname - xxxx = 6 LSB digits of MAC address

    Subscriptions:
      Ant/SW/ESP_xxxx/cmd - for sending commands to switch
        cmd = #0 - no antenna selected
        cmd = #1 - 8 - antenna 1 through 8 selected (only 1 at a time)
        cmd = #h or #H - halt automatic status updates
        cmd = #s or #S - start auto update and publish current switch status
                         now(updates all publications)
        cmd = #u - disable UDP mode
        cmd = #U - enable UDP mode
        cmd = st:xxxx - set status update rate xxxx = seconds (default = 60)
        cmd = sMQTT - reset MQTT connection

    Publications:
      Ant/SW/ESP_xxxx/will - last Will topic
      Ant/SW/ESP_xxxx/sw-state - current switch pin state
                                = hex value of switch control pins
      Ant/SW/ESP_xxxx/wifi-stats - current WiFi statistics
                      - timestamp(Unix seconds)
                      - SSID
                      - MAC address
                      - WiFi IP address
                      - Signal strength
                      - connection up time(seconds)
                      - # of reconnects
      Ant/SW/ESP_xxxx/mqtt-stats - current mqtt statistics
                      - timestamp(Unix seconds)
                      - current update rate(seconds)
                      - connection up time(seconds)
                      - # of resets
                      - # of reconnects
                      - # of commands received
                      - Avg Response time microSeconds
                      - # of messages sent
      Ant/SW/ESP_xxxx/log - command and error log

This version also has simplified telnet command interface on port 23
with straightforward 2 byte antenna set commands:
                      'A0;' - clears all antenna selections (grounded)
                      'A#;' - sets antenna # where # = 1 to 8
                      'AS;' = request status to be sent

// set band fields based on txFreq with resolution to 10 Hz

Band Decoder/Filter Controller Mode
-----------------------------------
Added functions and pins to control two Array Solutions Bandpasser II MBF-100 Bandpass Filter System devices

ESP8266 NodeMCU - ESP12E modules is used for Bandpasser II control

Radio 1 Filter GPIO - {12, 16, 4, 10}
Radio 2 Filter GPIO - {15, 5, 13, 14};

Bandpasser II - BCD code band control on Band Data pins A, B, C, D
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


** This sketch sets the "identity" of the controller to Antenna SW 1,
    Antenna SW 2 or band decoder based on the MAC Address of the ESP8266.
    See setup() for the current MAC address assignments.

** arduino_secrets.h must be updated for your WiFi SSID and Password
*****************************************************************************/

/* ############################################################################
 *
 * ver 19 - First version with Telnet and MQTT command methods
 *        - has OTA(Over The Air) update capability
 * ver 20 - Updated to include response time calculations for MQTT commands
 *        - currently DEBUG_TELNET, DEBUG_PUBS, and DEBUG_TIME is turned on
 *        - minor update to NTP Response serial message to include '\r'
 *        - changed the pubWiFiStats and publishStatus functions to send
 *          info similar to telnet status
 *        - tested with NR4O Antenna Controller v19proto and v20proto
 *        - deploying this version to real controllers
 * ver 21 - corrected pubWiFiStats SSID newline print
 *        - added myHostname to publishPins message
 *        - corrected build error with DEBUG on in mqttclient library on laptop
 *        - updated OTA hostname to myHostname
 * ver 214  - version modified for the LinkNode R4 relay board
 *          - version 21(4) LinkNode R4
 * ver 224  - refactored Telnet server client connect logic
 * ver 234  - working version with mulit-client telnet connect
 *          - telnet client read routine waits for \n - causes unwanted delays
 * ver 244  - Reworked Telnet command input logic for mulitiple clients
 *          - added o_Telnet_parseCmd.ino
 *          - added parseTelnet() function
 *          - added validInput() function
 *          - refactored telnetLoop() function
 *          - added checkNewClient() function
 *          - added checkClientInput() function
 *          - renamed printWiFiData to printSwitchData
 *          - antenna set functions now in parseTelnet
 *          - eliminated antenna set functions
 *          - update revision history for ver_num 244
 * ver 248  - update revision history for ver_num 248
 *          - changed enabled LINKNODE_R8 #define
 * ver 258  - udp protocol support prototype code
 *          - added #u and #U MQTT commands - disable/enable UDP mode
 *          - udp_mode defaults to enable(true)
 *          - added TinyXML parser and associated logic
 * ver 268  - added band decoder function
 *          - write band decoder pins whenever newData is received
 *          - default status interval changed to 30 seconds for pinState
 *          - WiFiStat and mqttStatus published 10x longer
 * ver 278  - added otrsp packet time out monitor
 *                - reset decoder if no udp packets received in 1 minute
 *          - added hostname debug printing to mqttWifi status
 *          - device mode to printed MQTT status
 * ver 288  - reworked the otrsp time out checks in udp_Loop.ino
 * ver 298  - corrected logic errors in decoder device after time out changes
 *          - fixed otrspTimer function
 * ver 308  - modified udp udpSetPins function for switch_1 and switch_2 logic
 * ver 318  - device mode now reported in pubWiFiStats
 * ver 328  - fixed myHostname set errors in case of Wifi reconnects
 * ver 338  - fixed switches resetting on otrspTimeOut
 *              - now only decoder resets when the timer expires
 * ver 348  - added a value check for fLen in the setBand() function
 *
 * ############################################################################
 */
 const uint16_t ver_num = 348;  // see update history for change log
                               // version 2x(4) LinkNode R4
                               // version 2x(8) LinkNode R8

// **** use this compiler directive for ESP8266 boards
// **** otherwise comment it out

//#define DEBUG_GEN   // print general debug messages to Serial
//#define DEBUG_GEN1
//#define DEBUG_TELNET
//#define DEBUG_GETTIME // prints getTime calculations to Serial
#define DEBUG_PUBS  // print publication debug messages to Serial
#define DEBUG_TIME  // print timer debug messages to Serial

//#define ESP8266     // Use NodeMCU board (not needed since board id'd as this type)
//#define LINKNODE_R4 // Use the LinkNode R4 board
#define LINKNODE_R8 // Use the LinkNode R8 board

#ifdef ESP8266
  #include <ESP8266WiFi.h> // for NodeMCU and ESP8266 ethernet modules
  #include <ESP8266WiFiMulti.h>
  // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
  ESP8266WiFiMulti wifiMulti;
#else
  #include <WiFiNINA.h> // for MKR1010 boards
  //#include <WiFi101.h>  // for MKR 1000 boards
  // Insert the equivalent WiFiMulti class declaration here
#endif

#include <ArduinoOTA.h>
#include <PString.h>
#include <ArduinoMqttClient.h>
#include <WiFiUdp.h>
#include <TinyXML.h>

//  **please enter your sensitive data in the Secret tab/arduino_secrets.h
#include "arduino_secrets.h"

// Globals used in the WiFi client routines
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

byte mac[6];  // the MAC address of your WiFi Module
IPAddress ip; // the IP address of this board
IPAddress subnetMask; // subnet mask
IPAddress gatewayIP; // IP address of the gateway
String hostname; // default hostname of this module as char array

// globals for the NTP time service functions
IPAddress timeServerIP;  // time.nist.gov NTP server IP Address
const char* NTPServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in first 48 bytes of the message
byte NTPBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming/outgoing packets

unsigned long intervalNTP = 600000 * 24UL; // Request NTP time every 24 hours
unsigned long prevNTP = 0;
unsigned long lastNTPResponse = millis();
uint32_t timeUNIX = 0;
unsigned long prevActualTime = 0;
bool flagNTP = false;

#ifdef ESP8266
  char rootHostname[20] = "ESP_";  // set root mqtt hostname of device
  char myHostname[20]; // char array for generated hostname at WiFi connect
#else
  char rootHostname[20] = "MKR1010_";  // set root mqtt hostname of device
  char myHostname[20]; // char array for generated hostname at WiFi connect
#endif

// To connect with SSL/TLS:
// 1) Change WiFiClient to WiFiSSLClient.
// 2) Change port value from 1883 to 8883.
// 3) Change broker value to a server with a known SSL/TLS root certificate
//    flashed in the WiFi module.

#ifdef LINKNODE_R4
  // initial variables for pin IO on LinkNode R4
  const int RELAYS = 4;
  const unsigned int aPin[] = {12, 16, 13, 14, 12, 16, 13, 14};
#else
  // initial variables for pin IO on LinkNode R8 & NodeMCU proto board
  const int RELAYS = 8;
  //const unsigned int LED = 12; // GPIO 12
  const unsigned int aPin[] = {12, 16, 4, 10, 15, 5, 13, 14};
#endif

int ledState = HIGH; // start with LED off
const unsigned int d1 = 2000; // first flash long on time
const unsigned int d2 = 1000; // off time
const unsigned int d3 = 500;  // second flash short on time

WiFiClient wifiClient; // create a WiFiClient object
WiFiUDP NTP_UDP;  // create udp object for the NTP client
WiFiUDP OTRSP_UDP;  // create udp object for the OTRSP xml data
unsigned int otrspPort = 12060; // deafault N1MM+ radio info port
MqttClient mqttClient(wifiClient);  // create a MqttClient object

// Globals used by the various MQTT client routines
const char broker[] =  "192.168.1.250";   //  broker host is NR4O-pi1 - otiginal assignment ="test.mosquitto.org"
int port = 1883;
const char username[] = "";
const char password[] = "";
const char topicPreamble[] = "Ant/SW/";
const char will[] = "/will";
const char cmd[] = "/cmd";
const char topic1[] = "/sw-state";
const char topic2[] = "/wifi=stats";
const char topic3[] = "/mqtt-stats";
const char topic4[] = "/log";
char willTopic[40] = "Ant/SW/ESP_xxxxxx/will";
char inTopic[40]   = "Ant/SW/ESP_xxxxxx/cmd";
char outTopic1[40]  = "Ant/SW/ESP_xxxxxx/sw-state";
char outTopic2[40] = "Ant/SW/ESP_xxxxxx/wifi-stats";
char outTopic3[40] = "Ant/SW/ESP_xxxxxx/mqtt-stats";
char outTopic4[40] = "Ant/SW/ESP_xxxxxx/log";
char msg[256] = "";  //message receive buffer
char bufOut[256] = ""; // buffer for message publication
PString msgOut(bufOut, sizeof(bufOut));
boolean newMsg = false;
const unsigned long keepAlive = 60 * 1000UL;  // milliseconds default = 60 * 1000L
const unsigned long timeout = 30 * 1000UL;  // milliseconds default = 30 * 1000L

// sets the MQTT pin state interval in mSec - other status is multiplier longer
unsigned long statInterval = 30 * 1000UL;
unsigned int statMultiplier = 10; // status delay multiplier
unsigned int statCount = 0;

unsigned long previousMillis = 0;
boolean glob_flash = false;
unsigned long cMillis = 0;
unsigned long p1Millis = 0;
unsigned long p2Millis = 0;

// connection count and time variables
unsigned long wifiConnectCount = 0;
unsigned long wifiStartTime = 0; // milliseconds since last WiFi connected
unsigned long wifiUpTime = 0;  // up time in seconds
unsigned long wifiCumUpTime = 0; // cumulative up time in seconds
unsigned long wifiAvgUpTime = 0; // average upt time in seconds
unsigned long mqttConnectCount = 0;
unsigned long mqttStartTime = 0; // milliseconds since last MQTT connected
unsigned long mqttUpTime = 0;  // up time in seconds
unsigned long mqttCumUpTime = 0; // cumulative up time in seconds
unsigned long mqttAvgUpTime = 0; // average up time in seconds
unsigned long msgRcvCount = 0;
unsigned long logCount = 0;
boolean statusFlag = true;  // flag to start stop sending status messages

int p = 0;  // pin counter
int count = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// globals for the Telnet server antenna switch command functions
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//how many clients should be able to telnet to this ESP8266

#define MAX_SRV_CLIENTS 2
#define MAX_LINE_LENGTH 128

// required network variables and objects
unsigned int localPort = 5555;
unsigned int telnetPort = 23;

WiFiServer server(telnetPort);
WiFiClient *serverClients[MAX_SRV_CLIENTS] = {NULL};
// char req[MAX_SRV_CLIENTS][MAX_LINE_LENGTH] = {0},{'\0'};
struct InputBuffer {
  uint8_t charCount;
  char inputLine[MAX_LINE_LENGTH];
};
struct InputBuffer InBuf[MAX_SRV_CLIENTS]; // declare an array of InputBuffers

// globals used to set HW configurations and roles based on myHostname
boolean switch_1 = false;
boolean switch_2 = false;
boolean decoder = false;

// Define global variables for Telnet server command functions

char buffer[1024];         // buffer for status message PString conversions
char s[128] = "" ;           // used for client reply formation
unsigned int l;           // temp variable for command buffer to string conversion
char r[128];               // buffer for command conversion to char string type
boolean wifiConnected = false;
boolean connectWifi();
PString msg_Status(buffer, sizeof(buffer));
String msg_Stat = "";

// resp_time function global timer variables
unsigned long rt_total = 0;
unsigned long num_req = 0;
unsigned long rt_max = 0;
unsigned long rt_min = 0;
unsigned long rt_avg = 0;
unsigned long rt_last = 0;
unsigned long t0 = 0; // initial time for telnet & MQTT command timer
unsigned long t1 = 0; // delta time 1
unsigned long t2 = 0; // delta time 2
unsigned long t3 = 0; // delta time 3
unsigned long t4 = 0; // delta time 4
unsigned long t5 = 0; // delta time 5 - used in response time calcuation
unsigned long t6 = 0; // delta time 6
unsigned long t7 = 0; // delta time 7

//++++ End Telnet controller required variables ++++++++++++++++++++++

// Global variables for the udp data XML parsing, radio#, frequency and
// antenna selection
boolean udp_mode = true;
TinyXML    xml;
uint8_t    xmlBuffer[1500]; // For XML decoding
char udpBuffer[1500];  // buffer for UDP reads - max MTU 1500
char replyBuffer[1500]; // buffer for replies
boolean xmlEnd = false;
unsigned long otrspMillis;
unsigned long otrspTimeOut = 30000UL; // 30 second activity time out
unsigned int otrspTimerExpired = 0;
unsigned int otrspTimerStatus = 0;
int deviceMode;  // variable to hold the mode device is operating in

// a struct defining a collection of key radio data
struct radioData {
  boolean       newData; // flag to indicate new data arrived
  char          txFreq[11];  // maximum 9 bytes of freq information up to 999.999999 MHz
  char          bandText[10]; // textual band ie 160M, 80M, etc
  uint16_t      bandNum; // integer value of band number
  byte          bandCode; // BCD code for Bandpasser II control
  uint16_t      antNum;
};

radioData radioTable[2]; // a table of radio data
unsigned int radioIndex = 0; // current radion number from xnl parser

void setup() {

radioTable[0].newData = false;
radioTable[1].newData = false;
  // initialize digital pins as an output.

#ifdef LINKNODE_R4
  // define 4 Relays on LINKNODE_R4 boards
  for (int i = 0; i < 4; i++) {
    pinMode(aPin[i], OUTPUT);
  }
#else
  // define 8 Relays on NodeMCU proto & LINKNODE_R8 boards
  for (int i = 0; i < 8; i++) {
    pinMode(aPin[i], OUTPUT);
  }
#endif

#ifdef LINKNODE_R4
  // Test LEDs by flashing each on/off/on briefly
  clearRelays();
  for (int i = 0; i < 4; i++) {
    digitalWrite(aPin[i], HIGH);
    delay(500);
    digitalWrite(aPin[i], LOW);
    delay(250);
  }
#else
  clearRelays();
  for (int i = 0; i < 8; i++) {
    digitalWrite(aPin[i], HIGH);
    delay(500);
    digitalWrite(aPin[i], LOW);
    delay(250);
  }
#endif

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  delay(1000);  // delay one second to wait for start

  msg[0] = '\0';  // initialize the incoming msg buffer with a NULL

  connectToWiFi();  // attempt to connect to WiFi network

  // set device mode as antenna switch_1, switch_2 or band decoder
  // set based on MAC address initially

  if (!strcasecmp(myHostname, "ESP_0604E9")) {
    deviceMode = 1;
    switch_1 = true;
    switch_2 = false;
    decoder = false;
  }
  else if (!strcasecmp(myHostname, "ESP_B18C18")) {
    deviceMode = 2;
    switch_1 = false;
    switch_2 = true;
    decoder = false;
  }
  else if (!strcasecmp(myHostname, "ESP_349ADA")) {
    deviceMode = 3;
    switch_1 = false;
    switch_2 = false;
    decoder = true;
  }
  else {
    deviceMode = 0;
    switch_1 = false;
    switch_2 = false;
    decoder = false;
  }

  // This next code block is setup for the Telnet command functions
  // start server

  server.begin();
  server.setNoDelay(true);

  localPort = telnetPort;
  Serial.println("");
  Serial.print("Ready! ... Use 'telnet ");
  Serial.print(WiFi.localIP());
  Serial.print(':');
  Serial.print(localPort);
  Serial.println("' to connect");
  Serial.println("");

  // ** End Telnet function setup

  startNTP_UDP();  // add udp to enable NTP server functions

  if(!WiFi.hostByName(NTPServerName, timeServerIP)) { // Get the IP address of the NTP timeServerIP
    Serial.println("DNS lookup failed. Rebooting.");
    Serial.flush();
    ESP.reset();
  }

  Serial.print("Time server IP: ");
  Serial.println(timeServerIP);
  unsigned long t1 = millis();  // get NTP start time

  while (!flagNTP && millis() - t1 < 30000UL) {  // continue if no response in 30 seconds
    delay(5);  // wait 5 milliseconds before checking for NTP time response
    getNTPtime();  // get current network time
  }
  unsigned long t2 = millis();  // get NTP responded time

  Serial.print("NTP response: ");
  Serial.println(timeUNIX);
  Serial.print("NTP response time: ");
  Serial.println(t2 - t1);  // print the NTP server response time


  // Start UDP server
  Serial.print("\nStarting UDP connection listener on ");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.println(otrspPort);

  // Start listening for UDP connections, report back via serial
  OTRSP_UDP.begin(otrspPort);
  otrspMillis = millis(); // initialize the timer to the current time

  // initialize xml parser
  xml.init((uint8_t *)udpBuffer, sizeof(buffer), &XML_callback);

  generateTopics();  // generate the MQTT topics w/myhostname

  // attempt to connect client to MQTT server
  connectMQTT();

  Serial.println("");
  Serial.println("***Starting loop()***\n");
}

void loop() {
  ArduinoOTA.handle();  // check for OTA updates

  getNTPtime();  // check for an NTP response and send a request every 24 hours

  telnetLoop();  // run the telnet command listener loop

  mqttLoop();   // run the MQTT listener function loop

  if (udp_mode) udpLoop();  // run the OTRSP udp listener function Loop

}
