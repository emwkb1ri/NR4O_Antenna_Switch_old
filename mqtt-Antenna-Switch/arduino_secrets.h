
#define SECRET_SSID "my-SSID"
#define SECRET_PASS "my-PASSWORD"

// alternate SSID
#define SECRET_SSID1 "my-ALT-SSID"
#define SECRET_PASS1 "my-ALT-PASSWORD"


/*LinkNode R8 GPIO Table - verified these against physical relay
 * that are unmarked on the board.
S1 - GPIO 12 - HIGH = ON
S2 - GPIO 16
S3 - GPIO 4
S4 - GPIO 10
S5 - GPIO 15
S6 - GPIO 5
S7 - GPIO 13
S8 - GPIO 14

LED/Relay ON pattern for LinkNode R8 - pattern of 8 LEDs
      Ant-6   Ant-3   Ant-8   Ant-1   = Antenna #
       D12     D11     D4      D10    = ESP8266/R8 digital output
       **      **      **      **     = LED

      Ant-5   Ant-4   Ant-7   Ant-2   = Antenna #
       D16     D18     D8      D3     = ESP8266/R8 digital output
       **      **      **      **     = LED
*/

/*LinkNode R4 GPIO Table - verified these against physical relay
 * that are unmarked on the board.
S2 - GPIO 16 - HIGH = ON
S3 - GPIO 14
S4 - GPIO 12
S5 - GPIO 13

LED/Relay ON pattern for LinkNode R8 - pattern of 8 LEDs
      Ant-4   Ant-3    = Antenna #
       S4      S2      = board switch #
       D10     D3      = ESP8266/R4 digital output
       **      **      = LED

      Ant-1   Ant-2   = Antenna #
       S3      S5     = board switch #
       D4      D8     = ESP8266/R4 digital output
       **      **     = LED
*/

/* the N1MM+ radio udp packet XML datagram format
udp_packet = """<?xml version="1.0" encoding="utf-8"?>
   <RadioInfo>
      <StationName>NR4O</StationName>
      <RadioNr>1</RadioNr>
      <Freq>2120000</Freq>
      <TXFreq>2120000</TXFreq>
      <Mode>CW</Mode>
      <OpCall>NR4O</OpCall>
      <IsRunning>False</IsRunning>
      <FocusEntry>00000</FocusEntry>
      <Antenna>4</Antenna>
      <Rotors>tribander</Rotors>
      <FocusRadioNr>1</FocusRadioNr>
      <IsStereo>False</IsStereo>
      <ActiveRadioNr>1</ActiveRadioNr>
   </RadioInfo>
"""
*/
