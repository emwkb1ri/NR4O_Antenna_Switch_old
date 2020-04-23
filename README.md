# mqtt-Antenna-Switch

WiFi/Telnet/MQTT controlled remote antenna switch relay controller using
LinkSprint LinkNode R4 & R8 ESP8266 controllers.

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
