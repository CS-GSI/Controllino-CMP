 /*
  cmp.cpp - This file implements the Controllino Message Protocol (CMP)
  CONTROLLINO is a programmable PLC based on arduino technology. See https://controllino.biz/
  CMP is intended to remotely control all digital, analog and Relay inputs and outputs via UDP datagram messages. 
  The current implementation focuses on the Controllino MEGA and must be adopted for different devices.
  CMP may however also be used as general purpose message protocol. 

  Created by C.Sauter, June 1, 2017.
  Released into the public domain.
*/

#include <Controllino.h>
#include <SPI.h>                 // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <stdio.h>
#include "cmp.h"

// Volatile memory to store random data
volatile int16_t volmem[CMP_SIZEOFVOLMEM];

void cmp::setup(IPAddress ip, byte * mac, unsigned int localPort){
  //cmp_setup(ip, mac, localPort);
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);

  #ifdef CMP_DEBUG
    Serial.print("Controllinio listens for UDP Datagramms on ");
    Serial.print(ip);
    Serial.print(":");
    Serial.println(localPort);
  #endif
  
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

static uint8_t cmp::cmp_getPinMode(uint8_t pin)
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

void cmp::core(){
  //cmp_core();  
  // buffers for receiving data
  char packetBuffer[UDP_TX_PACKET_MAX_SIZE+1];       
  // buffer for sending data
  char ReplyBuffer[UDP_TX_PACKET_MAX_SIZE+1] = "";   

    int packetSize = Udp.parsePacket();
  // if there's data available, read a packet
  if (packetSize) {
    #ifdef DEBUG
      DEBUG_PRINT("Received packet of size ");
      DEBUG_PRINTLN(packetSize);
      DEBUG_PRINT("From ");
      IPAddress remote = Udp.remoteIP();
      for (int i = 0; i < 4; i++) {
        Serial.print(remote[i], DEC);
        if (i < 3) {
          DEBUG_PRINT(".");
        }
      }
      DEBUG_PRINT(", port ");
      DEBUG_PRINTLN(Udp.remotePort());
    #endif

    // clear packet buffer
    memset(packetBuffer,0,sizeof(packetBuffer));
    memset(ReplyBuffer,0,sizeof(ReplyBuffer));
    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    #ifdef DEBUG
      DEBUG_PRINTLN("Contents:");
      DEBUG_PRINTLN(packetBuffer);
    #endif

    // Divide the datagram into its message parts
    char  V      = packetBuffer[CMP_OFFSET_VERSION];            // Version               'V': CMP Version
    char  R      = packetBuffer[CMP_OFFSET_REQRES];             // Request/Response      'S': Set, 'R': Read
    char  P      = packetBuffer[CMP_OFFSET_Property];           // Property              'A': Analog, 'C': Config, 'D': Digital, 'E': Error, 'I': Input, 'R': Relay, 'V': Volatile Memory
    char  N      = (packetBuffer[CMP_OFFSET_NUMBER]-48)*10      // NN: Number, ASCII 2 Byte to Char conversion.
                    + packetBuffer[CMP_OFFSET_NUMBER+1] - 48;   
    char  D      = packetBuffer[CMP_OFFSET_DATA];               // For Setting: 0:low, 1:high For Config I: Input, P: Pullup, O: Output
    char  PMODE  = (D == 'O') ? OUTPUT : ((D == 'P') ? INPUT_PULLUP : INPUT) ;      
    char  STATE  = (D == 'H') ? HIGH : LOW;
    int   MASK    = atoi(&(packetBuffer[CMP_OFFSET_DATA+1]));
    int   MEMMASK    = atoi(&(packetBuffer[CMP_OFFSET_DATA]));

    //  --------------- Invalid CMP Version -----------------
    if(V != CMP_VERSION){
      strcpy(ReplyBuffer, CMP_ERR_VERSIONMISMATCH);
    }
    //  --------------- Valid CMP Version -------------------
    else{
      strcpy(ReplyBuffer, packetBuffer);

      //  --------------- VS  SET ---------------------------
      if(R == 'S'){
        DEBUG_PRINTLN("R == S");

        //  --------------- VSC  SET Config ------------------
        if(P == 'C'){
          DEBUG_PRINTLN("P == C");

          ReplyBuffer[CMP_OFFSET_DATA] = (PMODE == OUTPUT) ? 'O' : ((PMODE == INPUT_PULLUP) ? 'P' : 'I');
          //ReplyBuffer[CMP_OFFSET_DATA+1] = 0;
        
          if(N == 99){
            for(int i = 0; i < sizeof(digitals); ++i){
              if((int)(1 << i) & MASK) pinMode(digitals[i], PMODE);
            }
          }
          else if (N >= 0 && N < sizeof(digitals)){
            pinMode(digitals[N], PMODE);
          }
          else{
            strcpy(ReplyBuffer, CMP_ERR_SCOPEDIGITALSNUM);
          }
        }
        //  --------------- VSD  SET Digital ------------------
        else if(P == 'D'){
          DEBUG_PRINTLN("P == D");

          ReplyBuffer[CMP_OFFSET_DATA] = (STATE == HIGH) ? 'H' : 'L';
          //ReplyBuffer[CMP_OFFSET_DATA+1] = 0;
           
          if(N == 99){
            for(int i = 0; i < sizeof(digitals); ++i){
              if((int)(1 << i) & MASK){
                pinMode(digitals[N], OUTPUT);
                digitalWrite(digitals[i], STATE);
              }
            }
          }
          else if (N >= 0 && N < sizeof(digitals)){
            pinMode(digitals[N], OUTPUT);
            digitalWrite(digitals[N], STATE);
          }
          else{
            strcpy(ReplyBuffer, CMP_ERR_SCOPEDIGITALSNUM);
          }
        }
        //  --------------- VSE  SET Error ----------------
        else if(P == 'E'){
          DEBUG_PRINTLN("P == E");
          
          // ....
        }
        //  --------------- VSR  SET Relay ------------------
        else if(P == 'R'){
          DEBUG_PRINTLN("P == R");

          ReplyBuffer[CMP_OFFSET_DATA] = (STATE == HIGH) ? 'H' : 'L';
          //ReplyBuffer[CMP_OFFSET_DATA+1] = 0;
          
          if(N == 99){
            for(int i = 0; i < sizeof(relays); ++i)
              if((int)(1 << i) & MASK) digitalWrite(relays[i], STATE);
          }
          else if (N >= 0 && N < sizeof(relays)){
            digitalWrite(relays[N], STATE);
          }
          else{
            strcpy(ReplyBuffer, CMP_ERR_SCOPERELAYNUM);
          }
        }
        //  --------------- VSV  SET Volatile Memory ----------
        else if(P == 'V'){
          DEBUG_PRINTLN("P == V");

          if((N >= 0) && (N < CMP_SIZEOFVOLMEM)){
            volmem[N] = MEMMASK;
            DEBUG_PRINTLN(MASK);
          }
          else {
            strcpy(ReplyBuffer, CMP_ERR_SCOPEREVOLMEM);
          }
        }
        else {
          strcpy(ReplyBuffer, CMP_ERR_PROPERTYUNKNOWN);
        }
      }

      //  --------------- VS  READ --------------------------
      else if(R == 'R'){
        DEBUG_PRINTLN("R == R");

        //  --------------- VRA  READ Analog ----------------
        if(P == 'A'){
          DEBUG_PRINTLN("P == A");
          
          ReplyBuffer[CMP_OFFSET_DATA] = 0;
          char str[15];
          
          if(N == 99){
            for(int i = 0; i < sizeof(analogs); ++i){
               
               if(i > 0) strcat(ReplyBuffer,".");
               
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
          else if ((N >= 0) && (N < sizeof(analogs))){
            uint16_t ar;
            for(int rr = 0; rr < 5; ++rr) ar = analogRead(analogs[N]);
            sprintf(str, "%u", ar);
            strcat(ReplyBuffer,str);
          }
          else{
            strcpy(ReplyBuffer, CMP_ERR_SCOPEANALOG);
          }
        }
        //  --------------- VRC  READ Config -----------------
        else if(P == 'C'){
          DEBUG_PRINTLN("P == C");
          
          if(N == 99){
            uint32_t bitpattern = 0;
            char str[sizeof(digitals)+1];
            str[sizeof(digitals)] = 0;
            DEBUG_PRINTLN(sizeof(digitals));
            for(int i = 0; i < sizeof(digitals); ++i){
              str[i] = cmp_getPinMode(digitals[i]);
            }
            ReplyBuffer[CMP_OFFSET_DATA] = 0;
            strcat(ReplyBuffer,str);
          }
          else if ((N >= 0) && (N < sizeof(digitals))){
            ReplyBuffer[CMP_OFFSET_DATA] = cmp_getPinMode(digitals[N]);
            ReplyBuffer[CMP_OFFSET_DATA+1] = 0;
          }
          else{
            strcpy(ReplyBuffer, CMP_ERR_SCOPEDIGITALSNUM);
          }
        }
        //  --------------- VRD  READ Digital ----------------
        else if(P == 'D'){
          DEBUG_PRINTLN("P == D");
          
          if(N == 99){
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
            ReplyBuffer[CMP_OFFSET_DATA] = 0;
            sprintf(str, "%u", highword);
            strcat(ReplyBuffer,str);
            strcat(ReplyBuffer,".");
            sprintf(str, "%u", lowword);
            strcat(ReplyBuffer,str);
          }
          else if ((N >= 0) && (N < sizeof(digitals))){
            ReplyBuffer[CMP_OFFSET_DATA] = (digitalRead(digitals[N]) == HIGH ? 'H' : 'L');
            ReplyBuffer[CMP_OFFSET_DATA+1] = 0;
          }
          else{
            strcpy(ReplyBuffer, CMP_ERR_SCOPEDIGITALSNUM);
          }
        }
        //  --------------- VRE  READ Error ----------------
        else if(P == 'E'){
          DEBUG_PRINTLN("P == E");
          
          // ....
        }
        //  --------------- VRI  READ Input ----------------
        else if(P == 'I'){
          DEBUG_PRINTLN("P == I");

          if(N == 99){
            char bitpattern = 0;  
            for(int i = 0; i < sizeof(inputs); ++i){
              bitpattern |= (digitalRead(inputs[i]) << i);
            }
            char str[15];
            sprintf(str, "%u", bitpattern);
            ReplyBuffer[CMP_OFFSET_DATA] = 0;
            strcat(ReplyBuffer,str);
          }
          else if (N == 0 || N == 1 || N == 16 || N == 17 || N == 18){
            int index = 0;
            if(N == 16) index = 0;
            else if(N == 17) index = 1;
            else if(N == 18) index = 2;
            else if(N == 0)  index = 3;
            else if(N == 1)  index = 4;
            
            ReplyBuffer[CMP_OFFSET_DATA] = (digitalRead(inputs[index]) == HIGH ? 'H' : 'L');
            ReplyBuffer[CMP_OFFSET_DATA+1] = 0;
          }
          else{
            strcpy(ReplyBuffer, CMP_ERR_SCOPEAINPUTS);
          }
        }
        //  --------------- VRR  READ Relay ----------------
        else if(P == 'R'){
          DEBUG_PRINTLN("P == R");
          
          uint16_t bitpattern = 0;
          
          DEBUG_PRINTLN(sizeof(relays));

          if(N == 99) {
            for(int i = 0; i < sizeof(relays); ++i){
              uint16_t rr = digitalRead(relays[i]);
              bitpattern |= (uint16_t)(rr << i);
            }
            #ifdef DEBUG
              Serial.print(bitpattern, BIN);
            #endif
            char str[15];
            ReplyBuffer[CMP_OFFSET_DATA] = 0;
            sprintf(str, "%u", bitpattern);
            strcat(ReplyBuffer,str);
          }
          else if ((N >= 0) && (N < sizeof(relays))){
            ReplyBuffer[CMP_OFFSET_DATA] = (digitalRead(relays[N]) == HIGH ? 'H' : 'L' );
            ReplyBuffer[CMP_OFFSET_DATA+1] = 0;
          }
          else{
            strcpy(ReplyBuffer, CMP_ERR_SCOPERELAYNUM);
          }
        }
        //  --------------- VRV  READ Volatile Memory --------
        else if(P == 'V'){
          DEBUG_PRINTLN("P == V");
          
          DEBUG_PRINTLN(CMP_SIZEOFVOLMEM);
          
          if((N >= 0) && (N < CMP_SIZEOFVOLMEM)){
            char str[15];
            ReplyBuffer[CMP_OFFSET_DATA] = 0;
            sprintf(str, "%+d", volmem[N]);
            strcat(ReplyBuffer,str);
          }
          else {
            strcpy(ReplyBuffer, CMP_ERR_SCOPEREVOLMEM);
          }
        }
        else {
          strcpy(ReplyBuffer, CMP_ERR_PROPERTYUNKNOWN);
        }
      }
      else {
        strcpy(ReplyBuffer, CMP_ERR_REQUNKNONW);
      }
    }
    
    ReplyBuffer[0] = CMP_VERSION;
    DEBUG_PRINT("ReplyBuffer : ");
    DEBUG_PRINTLN(ReplyBuffer);
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
  }
}
