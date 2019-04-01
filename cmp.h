/*
  cmp.h - This file is the header to the Controllino Message Protocol (CMP) implementation
  CONTROLLINO is a programmable PLC based on arduino technology. See https://controllino.biz/
  CMP is intended to remotely control all digital, analog and Relay inputs and outputs via UDP datagram messages. 
  The current implementation focuses on the Controllino MEGA and must be adopted for different devices.
  CMP may however also be used as general purpose message protocol. 

  Created by C.Sauter, June 1, 2017.
  Released into the public domain.
*/

#ifndef CMP_h
#define CMP_h

#define CMP_VERSION 'A'
#define CMP_OFFSET_VERSION  0
#define CMP_OFFSET_REQRES   1
#define CMP_OFFSET_Property 2
#define CMP_OFFSET_NUMBER   3
#define CMP_OFFSET_DATA     5

#define CMP_ERR_NOERR               "VEE00"
#define CMP_ERR_VERSIONMISMATCH     "VEE01"
#define CMP_ERR_SCOPEDIGITALSNUM    "VEE02"
#define CMP_ERR_SCOPERELAYNUM       "VEE03"
#define CMP_ERR_SCOPEREVOLMEM       "VEE04"
#define CMP_ERR_PROPERTYUNKNOWN     "VEE05"
#define CMP_ERR_SCOPEANALOG         "VEE06"
#define CMP_ERR_SCOPEAINPUTS        "VEE07"
#define CMP_ERR_REQUNKNONW          "VEE08"

//Debug decreases speed extremly, do not forget to disable it!
//#define CMP_DEBUG
 
#ifdef CMP_DEBUG
 #define DEBUG_PRINT(x)  Serial.print (x)
 #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTLN(x)
#endif

#define UNKNOWN_PIN 0xFF
#define CMP_SIZEOFVOLMEM 50

extern volatile int16_t volmem[];

// All relay outputs
const char relays[] = {    CONTROLLINO_R0,
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

// All digital inputs (input only)
const char inputs[] = {    CONTROLLINO_I16, 
                                CONTROLLINO_I17, 
                                CONTROLLINO_I18, 
                                CONTROLLINO_IN0, 
                                CONTROLLINO_IN1     };
// All analog inputs
const char analogs[] = {   CONTROLLINO_A0,
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
                          
// All digital input/outputs which are not input only
const char digitals[] = {  CONTROLLINO_D0,
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


class cmp {
  private:
    char packetBuffer[UDP_TX_PACKET_MAX_SIZE+1];      // buffers for receiving data 
    char ReplyBuffer[UDP_TX_PACKET_MAX_SIZE+1] = "";    // buffer for sending data
  
  public:
    void setup(IPAddress ip, byte * mac, unsigned int localPort);  
    void core();
    static uint8_t cmp_getPinMode(uint8_t pin);
	EthernetUDP Udp;
};

#endif
