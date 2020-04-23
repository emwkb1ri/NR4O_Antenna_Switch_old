void mqttLoop() {
    // call poll() regularly to allow the library to receive MQTT messages and
    // send MQTT keep alives which avoids being disconnected by the broker
    int mqttClientStatus = mqttClient.connected();
    if (!mqttClientStatus) {
      Serial.print(" MQTT NOT CONNECTED!...");
      Serial.print(" Status: ");
      Serial.print(mqttClientStatus);
      Serial.print("...Error Code: ");
      Serial.println(mqttClient.connectError());

      // on failure - calculate the average mqtt up time
      mqttUpTime = (millis() - mqttStartTime) / 1000UL;  // seconds
  	  mqttCumUpTime += mqttUpTime;  // seconds
      mqttAvgUpTime = mqttCumUpTime / (mqttConnectCount + 1);  // seconds

      Serial.print("MQTT Connection Uptime: ");
      Serial.print(dhms(mqttUpTime));
      Serial.print(" Sec");
      Serial.print("  Avg Up Time: ");
      Serial.print(dhms(mqttAvgUpTime));
      Serial.println(" seconds");

      int clientStatus = wifiClient.connected();
      if (!clientStatus) {
  	  Serial.print("wifiClient Disconnected...");
      Serial.print("Value: ");
  	  Serial.print(clientStatus);
  	  Serial.print("...WiFi Status: ");

    	  switch (WiFi.status()) {
    		  case WL_CONNECTION_LOST :
    			Serial.print("CONNECTION_LOST");
          Serial.println("...attempting WiFi reconnect");

          connectToWiFi();  // reconnect WiFi
    			break;

    		  case WL_DISCONNECTED :
    			Serial.print("DISONNECTED");
          Serial.println("...attempting WiFi reconnect");

          connectToWiFi();  // reconnect WiFi
    			break;

    		  case WL_CONNECTED :
    			Serial.print("CONNECTED");
    			break;

    		  case WL_NO_SHIELD :
    			Serial.print("NO_SHIELD");
    			break;

    		  case WL_NO_SSID_AVAIL :
    			Serial.print("NO_SSID_AVAIL");
    			break;

    		  case WL_CONNECT_FAILED :
    			Serial.print("CONNECT_FAILED");
          Serial.println("...attempting WiFi reconnect");

          connectToWiFi();  // reconnect WiFi
    			break;
    		  default:
    			// no matches to the above - must be one
    			//   WL_AP_CONNECTED, WL_AP_LISTENING, WL_IDLE_STATUS, WL_SCAN_COMPLETED
    			Serial.println("One of 4 other status returns");
    	  }
      }
      Serial.println();
      connectMQTT();  // attempt to reconnect to MQTT broker
      mqttStartTime = millis();
    }

    wifiUpTime = (millis() - wifiStartTime) / 1000UL;  // seconds
    mqttUpTime = (millis() - mqttStartTime) / 1000UL;  // seconds

    // ******** check for MQTT work to do *********
    mqttClient.poll();

    if (wifiConnectCount == 1) { // set the Avg Up Time to the current up time on first connection
      wifiAvgUpTime = wifiUpTime;  // seconds
      wifiCumUpTime = wifiUpTime;  // seconds
    }

    if (mqttConnectCount == 1) { // set the Avg Up Time to the current up time on first connection
      mqttAvgUpTime = mqttUpTime;  // seconds
      mqttCumUpTime = mqttUpTime;  // seconds
    }

    #ifdef DEBUG_GEN1
    Serial.print("newMsg = ");
    Serial.println(newMsg);
    #endif

    // on a new message parse the command and take action
    if (newMsg == true) {
      parseCmd();
    }

    // avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
    // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
    unsigned long currentMillis = millis();

    // publish pin status every statInterval - others 10x statInterval
    if ((currentMillis - previousMillis >= statInterval) && statusFlag) {
      // save the last time a message was sent
      previousMillis = currentMillis;

      publishPins();
      statCount++;  // increment the status counter

      // publish the WiFi and MQTT sttatus info every multiplier times
      if (statCount == statMultiplier) {
        pubStatus();
        statCount = 0; // reset status counter
      }
    }
}
