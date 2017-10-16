#include <Controllino.h>
//#include <SPI.h>                 // needed for Arduino versions later than 0018
//#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
//#include <stdio.h>
#include "cmp.h"

// Enter a MAC address and IP address for your controller below
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// IP address of your controllino
IPAddress ip(192, 168, 0, 12);
// Local port to listen on UDP datagram messages
unsigned int localPort = 8888;

void setup() {
  #ifdef CMP_DEBUG
    Serial.begin(9600);
  #endif
  cmp_setup(ip, mac, localPort);
}

void loop() {
  cmp_core();
  //delay(10);

  // User Program here
}


