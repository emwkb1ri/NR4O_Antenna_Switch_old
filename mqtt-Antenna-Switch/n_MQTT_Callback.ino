/*-----------------------------------------------------------------
/*            MQTT message callback routine
/*----------------------------------------------------------------*/
void onMqttMessage(int messageSize) {
  char buf[256];
  PString rcvOut(buf, sizeof(buf));
  // we received a message, print out the topic and contents
  // and make it available in the buffer called 'msg'
  #ifdef DEBUG_GEN1
  Serial.print("Received message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', DUP = ");
  Serial.print(mqttClient.messageDup() ? "true" : "false");
  Serial.print(", QoS = ");
  Serial.print(mqttClient.messageQoS());
  Serial.print(", Retain = ");
  Serial.print(mqttClient.messageRetain() ? "true" : "false");
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");
  #endif

  int i = 0;  // character array interator

  // use the stream interface to read the message data
  while (mqttClient.available()) {
    msg[i] = mqttClient.read();  // read each byte into buffer
    i = i + 1;
  }
  msg[i] = '\0';

  newMsg = true;
  msgRcvCount += 1;  // increment the received message counter
  t0 = micros();  // intialize time when command is received
  
  // publish a Command received log message
  rcvOut.begin();
  rcvOut.print(timeReadable(getTime(0)));
  rcvOut.print(" ");
  rcvOut.print("CMD= ");
  rcvOut.print(msg);

  publishLog(rcvOut);

  #ifdef DEBUG_GEN1
  Serial.print("CMD = ");
  Serial.println(msg); // print the message received
  #endif
}
