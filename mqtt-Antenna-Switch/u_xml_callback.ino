/* TinyXML invokes this callback as elements are parsed in XML,
allowing us to pick out just the tidbits of interest rather than
dynamically allocating and then traversing a whole brobdingnagian
tree structure; very Arduino-friendly!
As written here, this looks for XML tags named "/RadioInfo/RadioNr",
"/RadioInfo/TXFreq", "/RadioInfo/Antenna" and ending tag "/RadioInfo".
It extracts the tag data and stores it in a 2 element public struct table,
sets the newData flag for the respective element. When the end of the
xml datagram is detected, it sets the xmlEnd flag to stop processing data.
*/

void XML_callback(uint8_t statusflags, char* tagName,
 uint16_t tagNameLen, char* data, uint16_t dataLen) {
/*
  Serial.print(statusflags);
  Serial.print(" ");
  Serial.print(tagName);
  Serial.print(" ");
  Serial.println(data);
*/
  if((statusflags & STATUS_TAG_TEXT) && !strcasecmp(tagName, "/RadioInfo/RadioNr")) {
/*
    Serial.print("***");
    Serial.print(tagName); // print raw info
    Serial.print(" ");
    Serial.println(data);
*/
    radioIndex = atoi(data) - 1; // the RadioNr
    radioTable[radioIndex].newData = true;
  }
  if((statusflags & STATUS_TAG_TEXT) && !strcasecmp(tagName, "/RadioInfo/TXFreq")) {
/*
    Serial.print("***");
    Serial.print(tagName); // print raw info
    Serial.print(" ");
    Serial.println(data);
*/
    for (int i = 0; i < dataLen; i++){
      radioTable[radioIndex].txFreq[i] = data[i]; // copy to the radioTable
    }
    setBand(radioIndex, data, dataLen);
  }
  if((statusflags & STATUS_TAG_TEXT) && !strcasecmp(tagName, "/RadioInfo/Antenna")) {
/*
    Serial.print("***");
    Serial.print(tagName); // print raw info
    Serial.print(" ");
    Serial.println(data);
*/
    radioTable[radioIndex].antNum = atoi(data); // antenna number
  }

  if((statusflags & STATUS_END_TAG) && !strcasecmp(tagName, "/RadioInfo")) {
    // Serial.println("***End of RadioInfo datagram");
    xmlEnd = true;  // set end of message flag
  }
}
