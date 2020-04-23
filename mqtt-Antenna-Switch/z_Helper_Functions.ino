/*-------------------------------------------------------------------------
/*                  Function to set all relays off
/*  LOW = Relay OFF/proto LED OFF --- HIGH = Relay ON/proto LED ON
/*------------------------------------------------------------------------*/
void clearRelays() {
  #ifdef LINKNODE_R4
    for (int i = 0; i < 4; i++) {
      digitalWrite(atPin[i], LOW);
    }
  #else
  for (int i = 0; i < 8; i++) {
    digitalWrite(aPin[i], LOW);
  }
  #endif
}

String dhms(unsigned long s) {
  // return a string with Days: Hrs:  Min:  Sec: calculated from the unsigned long s seconds argument
  unsigned long secs = 0;
  unsigned long t_mins = 0;
  unsigned long mins = 0;
  unsigned long t_hrs = 0;
  unsigned long hrs = 0;
  unsigned long days = 0;
  String r = "";

  if (s < 1) {
      // no complete seconds have elapsed
      r = "0d: 0h: 0m: 0s:";
      return r;
  }

  secs = s % 60;    // remaining # of seconds

  if (s >= 60) {
    t_mins = (s - secs) / 60; // total minutes
    mins = t_mins % 60;  // remainder of minutes
  }
  if (t_mins >= 60) {
      t_hrs = (t_mins - mins) / 60; // total hours
      hrs = t_hrs % 24; // remainder of hours in days
  }
  if (t_hrs >= 24) {
      days = (t_hrs - hrs) / 24; // number of days
  }
  r += days;
  r += "d:";
  r += hrs;
  r += "h:";
  r += mins;
  r += "m:";
  r += secs;
  r += "s";
  return r;
}

/* ****************************************************************************
 *  function to calculate average command response time over last 1000 requests
 * ****************************************************************************
*/
void resp_time() {
  num_req++;       // increment the number of requests
  if (num_req > 1000) {  // if requests is greater than 1000 reset resp_time calc
    rt_total = 0;
    num_req = 1;
    rt_max = 0;
    rt_min = 0;
  }
  // validate switch response time for debugging
  t5 = micros() - t0;
  rt_last = t5; // set the current rt to the last for status reporting
  rt_total += t5;  // accumulate the response time
  rt_avg = rt_total/num_req;
  if(num_req == 1) rt_min = t5;
  if(t5 > rt_max) rt_max = t5;
  else if (t5 < rt_min || rt_min == 0) rt_min = t5;
}

/*--------------------------------------------------------------------------
/*                Array to HEX string function
/*------------------------------------------------------------------------*/
void array_to_string(byte array[], unsigned int len, char buffer[]) {
  // converts a byte array to a hex character string that can be printed
  // args are:
  //      byte array to be converted
  //      length of array
  //      destination char buffer

    for (unsigned int i = 0; i < len; i++){
        byte nib1 = (array[i] >> 4) & 0x0F;  // mask the first nibble of data in element i of array
        byte nib2 = (array[i] >> 0) & 0x0F;  // mask the second nibble of data in element i of array
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;  // convert the nibble to the ASCII character '0' to 'F'
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

/*---------------------------------------------------------------------
/*          Function to return MAC Address as a PString
/*-------------------------------------------------------------------*/
/*
String getMacAddress() {
  char* s[3] = {'\0', '\0', '\0'};
  char b[30];  // # of characters returned
  #ifdef ESP8266
  // swap order of bytes for ESP8266 modules
    for (unsigned int i = 0; i < 5; i++)
  #else
  // use this ordering for all others like MKR1010
    for (unsigned int i = 5; i > 0; i--)
  #endif

    {
      byte array = { mac[1] };
      array_to_string(array, 1, s);
      strcat(b, s);
      strcat(b, ":");
    }

  #ifdef ESP8266
    byte array = { mac[1] };
    array_to_string(array, 1, s);
    strcat(b, s);
  #else
    byte array = { mac[1] };
    array_to_string(array, 1, s);
    strcat(b, s);

  #endif
  return b;
}
*/
