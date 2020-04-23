/*-----------------------------------------------------------------------
/*                  MQTT Setup & Connect Function
/*---------------------------------------------------------------------*/
boolean  connectMQTT() {
  // Function to connect and reconnect as necessary to the MQTT server.
  // Should be called in the loop function and it will take care ff connecting.
  int8_t ret;
  PString msgOut(bufOut, sizeof(bufOut));
  PString willPayload(bufOut, sizeof(bufOut));

  // Stop if already connected.
  if (mqttClient.connected()) {
    Serial.println("MQTT already connected");
    return true;
  }

  // Each client must have a unique client ID
  mqttClient.setId(myHostname);

  // set the client keep alive and timeout intervals
  mqttClient.setKeepAliveInterval(keepAlive);
  mqttClient.setConnectionTimeout(timeout);

  // You can provide a username and password for authentication
  mqttClient.setUsernamePassword(username, password);

  // set a will message, used by the broker when the connection dies unexpectantly
  // you must know the size of the message before hand, and it must be set before connecting
  willPayload.begin();  // initialize the PString to the beginning
  willPayload.print("ERROR: ");
  willPayload.print(myHostname);
  willPayload.print(" - STOPPED RESPONDING");
  bool willRetain = true;
  int willQoS = 1;

  mqttClient.beginWill(willTopic, willPayload.length(), willRetain, willQoS);
  mqttClient.print(willPayload);
  mqttClient.endWill();

  Serial.println("Connecting to MQTT broker... ");

  // set the message receive callback
  mqttClient.onMessage(onMqttMessage);

  while(!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    Serial.println("Retrying MQTT connection in 5 seconds...");
    // mqttClient.disconnect();
    delay(5000);  // wait 5 seconds
  }

  mqttConnectCount++;  // increment the connect counter
  mqttStartTime = millis();  // initialize the mqtt start time

  Serial.println("MQTT Connected!");

  // assemble message and publish it - timestamp-Hostname-connected
  msgOut.begin();  // initialize the PString to the beginning
  msgOut.print("MQTT BROKER - CONNECTED");

  publishLog(msgOut);  // publish to the log topic

  Serial.print("Subscribing to topic: ");
  Serial.println(inTopic);
  Serial.println();

  // subscribe to the required topics
  // the second paramter set's the QoS of the subscription,
  // the the library supports subscribing at QoS 0, 1, or 2
  int subscribeQoS = 1;

  mqttClient.subscribe(inTopic, subscribeQoS);

  // assemble a message and publish it
  msgOut.begin();  // initialize the PString
  msgOut.print("Subscribed: ");
  msgOut.print(inTopic);

  publishLog(msgOut); // publish to the log topic

  // topics can be unsubscribed using:
  // mqttClient.unsubscribe(inTopic);

  Serial.print("Waiting for messages on topic: ");
  Serial.println(inTopic);
  Serial.println();

  return true;
} // end connectMQTT

/*-----------------------------------------------------------------------------
/*  This function generates the MQTT topics with myHostname embedded
/*----------------------------------------------------------------------------*/
void generateTopics() {
    // generate the will topic
    strcpy(willTopic, topicPreamble);
    strcat(willTopic, myHostname);
    strcat(willTopic, will);
    // generate the 'inbound' inTopic 'cmd'
    strcpy(inTopic, topicPreamble);
    strcat(inTopic, myHostname);
    strcat(inTopic, cmd);
    // generate outTopic1
    strcpy(outTopic1, topicPreamble);
    strcat(outTopic1, myHostname);
    strcat(outTopic1, topic1);
    // generate outTopic2
    strcpy(outTopic2, topicPreamble);
    strcat(outTopic2, myHostname);
    strcat(outTopic2, topic2);
    // generate outTopic3
    strcpy(outTopic3, topicPreamble);
    strcat(outTopic3, myHostname);
    strcat(outTopic3, topic3);
    // generate outTopic4
    strcpy(outTopic4, topicPreamble);
    strcat(outTopic4, myHostname);
    strcat(outTopic4, topic4);
}

/*---------------------------------------------------------------------------
/*            Function to publish all Stats
/*-------------------------------------------------------------------------*/
void pubStatus() {
  pubWiFiStats();
  publishStatus();
}

/*----------------------------------------------------------------------------
/*      Function to publish a time stamped MQTT Status message
/*--------------------------------------------------------------------------*/
void publishPins() {
  // create a time stamped message
  char buf[256];
  PString pinsOut(buf, sizeof(buf));

  // get pin state
  unsigned int pinState[8];
#ifdef LINKNODE_R4
  unsigned int pins = 4;
#else
  unsigned int pins = 8;
#endif

  // assemble pin state

  pinsOut.begin();
  pinsOut.print(myHostname);
  pinsOut.print(" - pinState = "); // remove this when program is working

  for (int i = 0; i < pins; i++) {
    pinState[i] = 0;
    pinState[i] = digitalRead(aPin[i]);
    pinsOut.print(pinState[i]);

    #ifdef DEBUG_GEN
      Serial.print("pinState[");
      Serial.print(i);
      Serial.print("] = ");
      Serial.println(pinState[i]);
    #endif
  }
  pinsOut.println();

  #ifdef DEBUG_PUBS
  Serial.print("Sending message to topic: ");
  Serial.println(outTopic1);
  Serial.println(pinsOut);
  #endif

  // send message, the Print interface can be used to set the message contents
  // in this case we know the size ahead of time, so the message payload can be streamed

  bool retained = false;
  int qos = 1;
  bool dup = false;

  // publish the message to outTopic3
  mqttClient.beginMessage(outTopic1, pinsOut.length(), retained, qos, dup);
  mqttClient.print(pinsOut);
  mqttClient.endMessage();

  // publish a log entry
  pinsOut.begin();
  pinsOut.print("Pin state published");
  publishLog(pinsOut);
}

/*----------------------------------------------------------------------------
/*        Function to publish a time stamped MQTT Status message
/*----------------------------------------------------------------------------
Topic: Ant/SW/ESP_xxxx/mqtt-stats - current mqtt statistics
MSG:            - timestamp(Unix seconds)
                - current update rate(seconds)
                - connection up time(seconds)
                - # of resets
                - # of reconnects
                - # of commands received
                - Avg Response time microSeconds
                - # of messages sent
----------------------------------------------------------------------------*/
void publishStatus() {
  // create a time stamped message
  char buf[256];
  PString statOut(buf, sizeof(buf));

  // assemble status
  statOut.begin();

  statOut.print("MQTT rcvd: ");
  statOut.print(msgRcvCount);
  statOut.print(" MQTT Reconnects: ");
  statOut.print(mqttConnectCount - 1);
  statOut.print(" MQTT Uptime: ");
  statOut.println(dhms(mqttUpTime));
  statOut.print("MQTT AVG Uptime: ");
  statOut.print(dhms(mqttAvgUpTime));
  statOut.print(" Update Interval: ");
  statOut.println(statInterval/1000UL);
  statOut.print("Requests: ");
  statOut.print(num_req);
  statOut.print("  Last: ");
  statOut.println(rt_last);
  statOut.print("Avg Resp Time: ");
  statOut.print(rt_avg);
  statOut.print("  Min: ");
  statOut.print(rt_min);
  statOut.print("  Max: ");
  statOut.println(rt_max);

  #ifdef DEBUG_PUBS
  Serial.print("Sending message to topic: ");
  Serial.println(outTopic3);
  Serial.println(statOut);
  #endif

  // send message, the Print interface can be used to set the message contents
  // in this case we know the size ahead of time, so the message payload can be streamed

  bool retained = false;
  int qos = 1;
  bool dup = false;

  // publish the message to outTopic3
  mqttClient.beginMessage(outTopic3, statOut.length(), retained, qos, dup);
  mqttClient.print(statOut);
  mqttClient.endMessage();

  // publish a log entry
  statOut.begin();
  statOut.print("Status published");
  publishLog(statOut);
}

/*---------------------------------------------------------------------------
/*      Function to publish an MQTT Log entry
/*-------------------------------------------------------------------------*/
void publishLog(PString s) {
  char buf[256];
  PString logOut(buf, sizeof(buf));
  unsigned long t = getTime(0);

  logOut.begin();
  logOut.print(timeReadable(t));  // get a time stamp for entry
  //logOut.print("TIMESTAMP3");
  logOut.print(" - ");
  logOut.print(myHostname);
  logOut.print(" - ");
  logOut.println(s);  // append log message

  #ifdef DEBUG_PUBS
  Serial.print("Sending message to topic: ");
  Serial.println(outTopic4);
  Serial.println(logOut);
  #endif

  // send message, the Print interface can be used to set the message contents
  // in this case we know the size ahead of time, so the message payload can be streamed

  bool retained = false;
  int qos = 1;
  bool dup = false;

  mqttClient.beginMessage(outTopic4, logOut.length(), retained, qos, dup);
  mqttClient.print(logOut);
  mqttClient.endMessage();

  logCount++;  // increment the log message counter
}

/*------------------------------------------------------------------------
/*              Function to Publish WiFi Statistics
/*------------------------------------------------------------------------
Topic: Ant/SW/ESP_xxxx/wifi-stats
MSG:  - timestamp(Unix seconds)
      - WiFi SSID
      - MAC address
      - WiFi IP address
      - myHostname
      - Signal strength
      - connection up time(seconds)
      - # of reconnects
/*---------------------------------------------------------------------*/
void pubWiFiStats() {
  char buf[256];
  char buf2[30];
  PString wifiOut(buf, sizeof(buf));
  PString macOut(buf2, sizeof(buf2));
  unsigned long t = getTime(0);

  macOut.begin();
  // print the MAC address beginning with byte 5 to byte 0
  macOut.print("MAC: ");
  #ifdef ESP8266
  // swap order of bytes for ESP8266 modules
    for (unsigned int i = 0; i < 5; i++)
  #else
  // use this ordering for all others like MKR1010
    for (unsigned int i = 5; i > 0; i--)
  #endif

    {
    macOut.print(mac[i], HEX);
    macOut.print(":");
    }

  #ifdef ESP8266
    macOut.print(mac[5], HEX);
  #else
    macOut.print(mac[0], HEX);
  #endif

  wifiOut.begin();
  wifiOut.print(timeReadable(t));  // get a time stamp for entry
  //wifiOut.print("TIMESTAMP4");
  wifiOut.print(" - Module Version: ");
  wifiOut.println(ver_num);
  wifiOut.print("Device mode: ");
  wifiOut.println(deviceMode);
  wifiOut.print("SSID: ");
  wifiOut.println(WiFi.SSID());
  wifiOut.print("HOST: ");
  wifiOut.print(myHostname);
  wifiOut.print(" / ");
  wifiOut.print(hostname);
  wifiOut.print("  IP: ");
  wifiOut.println(WiFi.localIP());
  wifiOut.print(macOut);
  wifiOut.print(" RSSI: ");
  wifiOut.print(WiFi.RSSI());
  wifiOut.println(" dBm");
  wifiOut.print("WiFi Reconnects: ");
  wifiOut.print(wifiConnectCount - 1);
  wifiOut.print("  WiFi Uptime: ");
  wifiOut.print(dhms(wifiUpTime));
  wifiOut.print("  WiFi AVG Uptime: ");
  wifiOut.println(dhms(wifiAvgUpTime));

  #ifdef DEBUG_PUBS
  Serial.print("Sending message to topic: ");
  Serial.println(outTopic2);
  Serial.println(wifiOut);
  #endif

  // send message, the Print interface can be used to set the message contents
  // in this case we know the size ahead of time, so the message payload can be streamed

  bool retained = false;
  int qos = 1;
  bool dup = false;

  mqttClient.beginMessage(outTopic2, wifiOut.length(), retained, qos, dup);
  mqttClient.print(wifiOut);
  mqttClient.endMessage();

  // publish a log entry
  wifiOut.begin();
  wifiOut.print("WiFi Stats published");
  publishLog(wifiOut);

}

/*---------------------------------------------------------------------------
/* Function used to flash an LED when an invalid message is received.
/*   +++This function is only for use in debugging on proto board
/*--------------------------------------------------------------------------*/

void flash(String s) {
  // used to flash LED8 quickly for 5 seconds when an invalid MQTT message
  // is recieved.

  const unsigned long i = 200;   // 300 mSec timer
  const unsigned long j = 5000;  // 5 second timer

  if (s == "Start") {
    // initialize the timer count variables if called with "Start"
    Serial.println("Start LED flash");
    glob_flash = true;
    cMillis = millis();
    p1Millis = cMillis;
    p2Millis = cMillis;
    ledState = LOW; //turn on the LED to start
  }
  cMillis = millis();
  // if glob_flash is true - fast flash at i mSec intervals for j mSec of time
  if (glob_flash && (cMillis - p1Millis >= i)) {
    p1Millis = cMillis;
    // Serial.println(cMillis, p1Millis);
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
  if (glob_flash && (cMillis - p2Millis >= j)) {
    Serial.println("Stop LED flash");
    Serial.println();
    ledState = HIGH; // turn the LED off
    glob_flash = false; // stop the fast flashing
  }
  // set the LED with the ledState of the variable
  digitalWrite(aPin[2], ledState);
  }
}
