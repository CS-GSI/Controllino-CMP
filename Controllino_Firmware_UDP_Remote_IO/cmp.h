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

//Debug decreases Speed extremly, do not forget to disable it!
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
//extern const char relays[];
//extern const char inputs[];
//extern const char analogs[];
//extern const char digitals[];

void cmp_setup(IPAddress ip, byte * mac, unsigned int localPort);
void cmp_core();
uint8_t cmp_getPinMode(uint8_t pin);

#endif
