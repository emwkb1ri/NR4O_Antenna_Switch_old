/*-----------------------------------------------------------------------
/*            Function to return readable time stamp
/*---------------------------------------------------------------------*/
PString timeReadable(unsigned long epoch) {
  // Calculates the Day, Month, Year, Hour, Mins, Secs
  // and returns a String of the form YYYYMMMDD-HH:MM:SS(.sss)
  // figure out how to add milliseconds later
  char buf[30];
  PString tStamp(buf, sizeof(buf));  // initialize return String
  String MMM = "";

  static unsigned char month_days[12]={31,28,31,30,31,30,31,31,30,31,30,31};
  static unsigned char week_days[7] = {4,5,6,0,1,2,3};
  //Thu=4, Fri=5, Sat=6, Sun=0, Mon=1, Tue=2, Wed=3

  unsigned char
  ntp_hour, ntp_minute, ntp_second, ntp_week_day, ntp_date, ntp_month, leap_days, leap_year_ind ;

  unsigned short temp_days;

  unsigned int
  ntp_year, days_since_epoch, day_of_year;

  char key;

     //---------------------------- Input and Calculations -------------------------------------
      leap_days=0;
      leap_year_ind=0;

      // Add or substract time zone here.
      // epoch+=19800 ; //GMT +5:30 = +19800 seconds

        ntp_second = epoch%60;
        epoch /= 60;
        ntp_minute = epoch%60;
        epoch /= 60;
        ntp_hour  = epoch%24;
        epoch /= 24;

        days_since_epoch = epoch;      //number of days since epoch
        ntp_week_day = week_days[days_since_epoch%7];  //Calculating WeekDay

        ntp_year = 1970+(days_since_epoch/365); // ball parking year, may not be accurate!

        int i;
        for (i=1972; i<ntp_year; i+=4)      // Calculating number of leap days since epoch/1970
           if(((i%4==0) && (i%100!=0)) || (i%400==0)) leap_days++;

        ntp_year = 1970+((days_since_epoch - leap_days)/365); // Calculating accurate current year by (days_since_epoch - extra leap days)
        day_of_year = ((days_since_epoch - leap_days)%365)+1;

        if(((ntp_year%4==0) && (ntp_year%100!=0)) || (ntp_year%400==0))
         {
           month_days[1]=29;     //February = 29 days for leap years
           leap_year_ind = 1;    //if current year is leap, set indicator to 1
          }
              else month_days[1]=28; //February = 28 days for non-leap years

              temp_days=0;

      for (ntp_month=0 ; ntp_month <= 11 ; ntp_month++) //calculating current Month
         {
             if (day_of_year <= temp_days) break;
             temp_days = temp_days + month_days[ntp_month];
          }

      temp_days = temp_days - month_days[ntp_month-1]; //calculating current Date
      ntp_date = day_of_year - temp_days;

     // -------------------- Printing Results -------------------------------------
  #ifdef DEBUG_GETTIME
      switch(ntp_week_day) {

                           case 0: Serial.print("\nSunday");
                                   break;
                           case 1: Serial.print("\nMonday");
                                   break;
                           case 2: Serial.print("\nTuesday");
                                   break;
                           case 3: Serial.print("\nWednesday");
                                   break;
                           case 4: Serial.print("\nThursday");
                                   break;
                           case 5: Serial.print("\nFriday");
                                   break;
                           case 6: Serial.print("\nSaturday");
                                   break;
                           default: break;
                           }
    Serial.print("-");
  #endif

    switch(ntp_month) {

                           case 1: MMM = "JAN";
                                   break;
                           case 2: MMM = "FEB";
                                   break;
                           case 3: MMM = "MAR";
                                   break;
                           case 4: MMM = "APR";
                                   break;
                           case 5: MMM = "MAY";
                                   break;
                           case 6: MMM = "JUN";
                                   break;
                           case 7: MMM = "JUL";
                                   break;
                           case 8: MMM = "AUG";
                                   break;
                           case 9: MMM = "SEP";
                                   break;
                           case 10: MMM = "OCT";
                                   break;
                           case 11: MMM = "NOV";
                                   break;
                           case 12: MMM = "DEC";
                           default: break;
                           }
  #ifdef DEBUG_GETTIME
    Serial.print(MMM);
    Serial.print("-");
    Serial.print(ntp_date);
    Serial.print("-");
    Serial.println(ntp_year);
    Serial.print("TIME = ");
    Serial.print(ntp_hour);
    Serial.print(":");
    Serial.print(ntp_minute);
    Serial.print(":");
    Serial.println(ntp_second);
    Serial.print("Days since Epoch: ");
    Serial.println(days_since_epoch);
    Serial.print("Number of Leap days since EPOCH: ");
    Serial.println(leap_days);
    Serial.print("Day of year = ");
    Serial.println(day_of_year);
    Serial.print("Is Year Leap? ");
    Serial.println(leap_year_ind);
  #endif

    // assemble time stamp PString
    tStamp.print(ntp_year);
    tStamp.print("-");
    tStamp.print(MMM);
    tStamp.print("-");
    tStamp.print(ntp_date);
    tStamp.print(" ");
    tStamp.print(ntp_hour);
    tStamp.print(":");
    tStamp.print(ntp_minute);
    tStamp.print(":");
    tStamp.print(ntp_second);
    //tStamp += " ";

    //printf("===============================================\n");
    //printf(" Press e to EXIT, or any other key to repeat...\n\n");
    //key=getch();
    //if (key!='e') goto start;
    return tStamp;
}

/*---------------------------------------------------------------------------
/*            Function to get current Time in seconds
/*
/* This currently simply returns time since the device powered up.
/* eventually this will get updated to include time from NTP source.
/*-------------------------------------------------------------------------*/
unsigned long getTime(int r) {
  // r = 0 seconds resolution
  // r = 1 milliseconds resolution
  // r = 2 microseconds resolution
  unsigned long actualTime = timeUNIX + (millis() - lastNTPResponse) / 1000UL;
  switch (r) {
     case 0:  // return time in seconds since UNIX Epoch
      return actualTime;
      break;
    case 1:   // return time in milliseconds since UNIX Epock - (may rollover)
      return actualTime = (timeUNIX * 1000UL) + (millis() - lastNTPResponse);
      break;
    case 2:  // this is not right due to overflow - maybe eliminate this
      return micros();
      break;
    default:
      return actualTime;
  }
}
