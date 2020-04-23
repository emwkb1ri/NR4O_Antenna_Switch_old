/*---------------------------------------------------------------
/*              Function to connect to WiFi
/*--------------------------------------------------------------*/
void connectToWiFi() {

  // attempt to connect to Wifi network:
  Serial.print("\n\n\rAttempting to connect to WPA SSID: ");
  Serial.println(ssid);
  WiFi.macAddress(mac);

  char str[10] = "";
  char buf[30];

  #ifdef ESP8266
    WiFi.mode(WIFI_STA);  // needed for ESP8266WiFi
    byte array[3] = { mac[3], mac[4], mac[5] };
  #else
    byte array[3] = { mac[2], mac[1], mac[0] };
  #endif
  // Generate a Hostname for this device based on myHostname plus the last byte of the MAC address
  strcpy(myHostname, rootHostname);  // set the root string for myHostname
  strcpy(buf, myHostname);
  //strcat(buf, ":");

  array_to_string(array, 3, str);  // convert the last 3 bytes of MAC address to a string
  strcat(buf, str);
  Serial.println(buf);

  if (wifiConnectCount != 0) { // calculate reconnect stats
    wifiUpTime = millis() - wifiStartTime / 1000UL;  // seconds
    wifiCumUpTime += wifiUpTime; // seconds
    wifiAvgUpTime = wifiCumUpTime / (wifiConnectCount + 1); // seconds
  }
  // initialize the WiFi library network settings
  // WiFi.begin(ssid, pass);
  wifiMulti.addAP(SECRET_SSID, SECRET_PASS); // add Wi-Fi networks you want to connect to
  wifiMulti.addAP(SECRET_SSID1, SECRET_PASS1);

  /* original single WiFi AP connection while loop
  /*while (WiFi.status() != WL_CONNECTED) {
    */
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) {
    // failed, retry
    Serial.print("*");
    delay(3000);
    // if not connected in 3*i seconds reset the ESP device
    if (i > 20) {
      ESP.reset();  // connection failed - reset device and retry
    }
    i++;
  }

  wifiConnectCount++; // increment the wifi connect count for Stats
  wifiStartTime = millis();  // initialize the wifi start time

  Serial.println("You're connected to the network");
  Serial.println();
  // print the firmware version of the WiFi module
  // Serial.print("Firmware Version: ");
  // Serial.println(WiFi.firmwareVersion());  // not valid for ESP8266WiFi

  // print the MAC address beginning with byte 5 to byte 0
  Serial.print("MAC: ");
  #ifdef ESP8266
  // swap order of bytes for ESP8266 modules
    for (unsigned int i = 0; i < 5; i++)
  #else
  // use this ordering for all others like MKR1010
    for (unsigned int i = 5; i > 0; i--)
  #endif

    {
    Serial.print(mac[i], HEX);
    Serial.print(":");
    }

  #ifdef ESP8266
    Serial.println(mac[5], HEX);
  #else
    Serial.println(mac[0], HEX);
  #endif

  // print the SSID you are connected to
  Serial.print("CONNECTED TO: ");
  Serial.println(WiFi.SSID());
  // print the signal strength of the WiFi
  Serial.print("Signal Strength (RSSI): ");
  Serial.println(WiFi.RSSI());

#ifdef ESP8266
  // hostname function only available on ESP8266WiFi
  hostname = (WiFi.hostname());  // get the current Hostname
  strcpy(myHostname, buf);
#else
  hostname = "NOT READABLE";
  strcpy(myHostname, buf);
#endif

  Serial.print("Hostname: ");
  Serial.print(hostname);
  Serial.print("  myHostname: ");
  Serial.println(myHostname);

  // print this device's ip address
  ip = WiFi.localIP();
  Serial.print("IP: ");
  Serial.println(ip);
  subnetMask = WiFi.subnetMask(); // get current subnet mask
  Serial.print("Subnet Mask: ");
  Serial.print(subnetMask);
  gatewayIP = WiFi.gatewayIP();  // get current gateway IP
  Serial.print("  Gateway: ");
  Serial.println(gatewayIP);

  // Now setup the Over The Air(OTA) update capability

  ArduinoOTA.setHostname(myHostname);
  ArduinoOTA.setPassword("NR4O");

  ArduinoOTA.onStart([]() {
      Serial.println("Start");
  });

  ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready");

} // end connectToWiFi
