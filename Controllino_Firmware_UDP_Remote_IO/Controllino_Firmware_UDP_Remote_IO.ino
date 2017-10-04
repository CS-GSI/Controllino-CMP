#include <Controllino.h>
#include <SPI.h>                 // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <stdio.h>
#include "cmp.h"

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 0, 12);
unsigned int localPort = 8888;      // local port to listen on

//Debug decreases Speed extremly, do not forget to disable it!
#define DEBUG

#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.print (x)
 #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTLN(x)
#endif

#define UNKNOWN_PIN 0xFF

uint8_t getPinMode(uint8_t pin)
{
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);

  // I don't see an option for mega to return this, but whatever...
  if (NOT_A_PIN == port) return UNKNOWN_PIN;

  // Is there a bit we can check?
  if (0 == bit) return UNKNOWN_PIN;

  // Is there only a single bit set?
  if (bit & bit - 1) return UNKNOWN_PIN;

  volatile uint8_t *reg, *out;
  reg = portModeRegister(port);
  out = portOutputRegister(port);

  if (*reg & bit)
    //return OUTPUT;
    return 'O';
  else if (*out & bit)
    //return INPUT_PULLUP;
    return 'P';
  else
    //return INPUT;
    return 'I';
}

volatile int16_t volmem[CMP_SIZEOFVOLMEM];

const char relays[] = {   CONTROLLINO_R0,
                          CONTROLLINO_R1, 
                          CONTROLLINO_R2,
                          CONTROLLINO_R3,
                          CONTROLLINO_R4,
                          CONTROLLINO_R5,
                          CONTROLLINO_R6,
                          CONTROLLINO_R7,
                          CONTROLLINO_R8,
                          CONTROLLINO_R9,
                          CONTROLLINO_R10,
                          CONTROLLINO_R11,
                          CONTROLLINO_R12,
                          CONTROLLINO_R13,
                          CONTROLLINO_R14,
                          CONTROLLINO_R15     };
                          
const char inputs[] = {   CONTROLLINO_I16, 
                          CONTROLLINO_I17, 
                          CONTROLLINO_I18, 
                          CONTROLLINO_IN0, 
                          CONTROLLINO_IN1     };

const char analogs[] = {  CONTROLLINO_A0,
                          CONTROLLINO_A1, 
                          CONTROLLINO_A2,
                          CONTROLLINO_A3,
                          CONTROLLINO_A4,
                          CONTROLLINO_A5,
                          CONTROLLINO_A6,
                          CONTROLLINO_A7,
                          CONTROLLINO_A8,
                          CONTROLLINO_A9,
                          CONTROLLINO_A10,
                          CONTROLLINO_A11,
                          CONTROLLINO_A12,
                          CONTROLLINO_A13,
                          CONTROLLINO_A14,
                          CONTROLLINO_A15     };

const char digitals[] = { CONTROLLINO_D0,
                          CONTROLLINO_D1, 
                          CONTROLLINO_D2,
                          CONTROLLINO_D3,
                          CONTROLLINO_D4,
                          CONTROLLINO_D5,
                          CONTROLLINO_D6,
                          CONTROLLINO_D7,
                          CONTROLLINO_D8,
                          CONTROLLINO_D9,
                          CONTROLLINO_D10,
                          CONTROLLINO_D11,
                          CONTROLLINO_D12,
                          CONTROLLINO_D13,
                          CONTROLLINO_D14,
                          CONTROLLINO_D15,
                          CONTROLLINO_D16,
                          CONTROLLINO_D17,
                          CONTROLLINO_D18,
                          CONTROLLINO_D19     };
                                                    

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
char ReplyBuffer[255] = "";                 // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void setup() {
  // start the Ethernet and UDP:
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);

  Serial.begin(9600);
  Serial.print("Controllinio listens for UDP Datagramms on ");
  Serial.print(ip);
  Serial.print(":");
  Serial.println(localPort );
  
  DEBUG_PRINTLN(sizeof(inputs));
  // inputs as inputs
  for (int i = 0; i < sizeof(inputs); ++i){
    pinMode(inputs[i], INPUT);
  }
  // relays as outputs
  for (int i = 0; i < sizeof(relays); ++i){
    pinMode(relays[i], OUTPUT);
  }
  
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    DEBUG_PRINT("Received packet of size ");
    DEBUG_PRINTLN(packetSize);
    DEBUG_PRINT("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i = 0; i < 4; i++) {
      #ifdef DEBUG
      Serial.print(remote[i], DEC);
      #endif
      if (i < 3) {
        DEBUG_PRINT(".");
      }
    }
    DEBUG_PRINT(", port ");
    DEBUG_PRINTLN(Udp.remotePort());

    // clear packet buffer
    for(int i = 0; i < UDP_TX_PACKET_MAX_SIZE; ++i)
      packetBuffer[i] = 0;
    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    DEBUG_PRINTLN("Contents:");
    DEBUG_PRINTLN(packetBuffer);

    
    strcpy(ReplyBuffer, "NAK: Command Invalid");

    char SRC    = packetBuffer[0];    // SRC    S: Set, R:Read, C: Config Pin
    char P      = packetBuffer[1];    // P      A: Analog, D: Digital, R: Relay, I: Input, V: Volatile Memory
                                      // N      Number, ASCII 2 Byte to Char conversion.
                                      //        Only for RA, SD, CD, SR
                                      //        RI, RD: combined readout
    char N      = (packetBuffer[2]-48)*10 + packetBuffer[3] - 48;   
    char D      = packetBuffer[4];    // For Setting: 0:low, 1:high For Config I: Input, P: Pullup, O: Output
    char PMODE  = (D == 'O') ? OUTPUT : ((D == 'P') ? INPUT_PULLUP : INPUT) ;      
    char STATE  = (D-48 == 1) ? HIGH : LOW;
    int MASK    = atoi(packetBuffer+5);
    
    if(packetSize == 5 || P == 'V' || N == 99){
      strcpy(ReplyBuffer, packetBuffer);
      
      if(SRC == 'S'){
        DEBUG_PRINTLN("SRC == S");
        if(P == 'R' && (N < sizeof(relays) || N == 99)){
          DEBUG_PRINTLN("P == R");
          if(N == 99){
            for(int i = 0; i < sizeof(relays); ++i)
              if((int)(1 << i) & MASK) digitalWrite(relays[i], STATE);
          }
          else {
            digitalWrite(relays[N], STATE);
          }
          ReplyBuffer[4] = (STATE == HIGH) ? '1' : '0';
        }
        else if(P == 'D' && (N < sizeof(digitals) || N == 99)){
          DEBUG_PRINTLN("P == D");
          if(N == 99){
            for(int i = 0; i < sizeof(digitals); ++i){
              if((int)(1 << i) & MASK){
                pinMode(digitals[N], OUTPUT);
                digitalWrite(digitals[i], STATE);
              }
            }
          }
          else {
            pinMode(digitals[N], OUTPUT);
            digitalWrite(digitals[N], STATE);
          }
          ReplyBuffer[4] = (STATE == HIGH) ? '1' : '0';
        }
        else if(P == 'C' && (N < sizeof(digitals) || N == 99)){
          DEBUG_PRINTLN("P == C");
          
          if(N == 99){
            for(int i = 0; i < sizeof(digitals); ++i){
              if((int)(1 << i) & MASK) pinMode(digitals[i], PMODE);
            }
          }
          else {
            pinMode(digitals[N], PMODE);
          }
          ReplyBuffer[4] = (PMODE == OUTPUT) ? 'O' : ((PMODE == INPUT_PULLUP) ? 'P' : 'I');
        }
        else if(P == 'V' && (N < CMP_SIZEOFVOLMEM)){
          DEBUG_PRINTLN("P == V");
          //int d = atoi(packetBuffer+4);
          // magic d ...
          volmem[N] = MASK;
          DEBUG_PRINTLN(MASK);
        }
        else {
          strcpy(ReplyBuffer, "NAK: Command Invalid");
        }
      }
      else if(SRC == 'R'){
        DEBUG_PRINTLN("SRC == R");
        if(P == 'A' && (N < sizeof(analogs) || N == 99)){
          DEBUG_PRINTLN("P == A");
          ReplyBuffer[4] = 0;
          char str[15];
          if(N == 99){
            for(int i = 0; i < sizeof(analogs); ++i){
               strcat(ReplyBuffer,".");
               
               uint16_t ar;
               for(int rr = 0; rr < 5; ++rr) ar = analogRead(analogs[i]);
               for(int i = 0; i < 15; ++i) str[i] = 0;
               sprintf(str, "%u", ar);
               DEBUG_PRINTLN(str);
               char highbyte = (ar >> 8);
               char lowbyte  = (char)ar;
               strcat(ReplyBuffer,str);
            }
          }
          else{
            uint16_t ar;
            for(int rr = 0; rr < 5; ++rr) ar = analogRead(analogs[N]);
            sprintf(str, "%u", ar);
            strcat(ReplyBuffer,str);
          }
          
          //for(int i = )
          //uint16_t ar = analogRead(analogs[N]);
          //char str[15];
          //sprintf(str, "%u", ar);
          //DEBUG_PRINTLN(str);
          //char highbyte = (ar >> 8);
          //char lowbyte = (char)ar;

          
          //ReplyBuffer[4] = 0;
          //strcat(ReplyBuffer,str);
        }
        else if(P == 'I'){
          DEBUG_PRINTLN("P == I");
          char bitpattern = 0;  
          for(int i = 0; i < sizeof(inputs); ++i){
            bitpattern |= (digitalRead(inputs[i]) << i);
          }
          char str[15];
          sprintf(str, "%u", bitpattern);
          ReplyBuffer[2] = 0;
          strcat(ReplyBuffer,str);
        }
        else if(P == 'D'){
          DEBUG_PRINTLN("P == D");
          uint32_t bitpattern = 0;
          DEBUG_PRINTLN(sizeof(digitals));
          for(int i = 0; i < sizeof(digitals); ++i){
            uint32_t dr = digitalRead(digitals[i]);
            bitpattern |= (uint32_t)(dr << i);
          }
          uint16_t highword = (bitpattern >> 16);
          uint16_t lowword  = (uint16_t)bitpattern;
          #ifdef DEBUG
          Serial.print(highword, BIN);
          Serial.println(lowword, BIN);
          #endif
          char str[15];
          ReplyBuffer[2] = 0;
          sprintf(str, "%u", highword);
          strcat(ReplyBuffer,str);
          strcat(ReplyBuffer,".");
          sprintf(str, "%u", lowword);
          strcat(ReplyBuffer,str);
        }
        else if(P == 'R'){
          DEBUG_PRINTLN("P == R");
          uint16_t bitpattern = 0;
          DEBUG_PRINTLN(sizeof(relays));
          for(int i = 0; i < sizeof(relays); ++i){
            uint16_t rr = digitalRead(relays[i]);
            bitpattern |= (uint16_t)(rr << i);
          }
          #ifdef DEBUG
          Serial.print(bitpattern, BIN);
          #endif
          char str[15];
          ReplyBuffer[2] = 0;
          sprintf(str, "%u", bitpattern);
          strcat(ReplyBuffer,str);
        }
        else if(P == 'V'){
          DEBUG_PRINTLN("P == V");
          DEBUG_PRINTLN(CMP_SIZEOFVOLMEM);
          char str[15];
          ReplyBuffer[4] = 0;
          sprintf(str, "%u", volmem[N]);
          strcat(ReplyBuffer,str);
        }
        else if(P == 'C'){
          DEBUG_PRINTLN("P == C");
          uint32_t bitpattern = 0;
          char str[sizeof(digitals)+1];
          str[sizeof(digitals)] = 0;
          DEBUG_PRINTLN(sizeof(digitals));
          for(int i = 0; i < sizeof(digitals); ++i){
            str[i] = getPinMode(digitals[i]);
          }
          ReplyBuffer[2] = 0;
          strcat(ReplyBuffer,str);
        }
        else {
          strcpy(ReplyBuffer, "NAK: Command Invalid");
        }
      }
    }
    // send a reply to the IP address and port that sent us the packet we received
    DEBUG_PRINT("ReplyBuffer : ");
    DEBUG_PRINT(ReplyBuffer);
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
  }
  //delay(10);

  // User Program here

}


