# Controllino-CMP
This is a Arduino library (cpp and header file) written to provide a message interface on Controllino Mega devices.

There is a LabVIEW client available [LV-Controllino-CMP (https://github.com/CS-GSI/LV-Controllino-CMP.git)](https://github.com/CS-GSI/LV-Controllino-CMP.git).

# HowTo use in your Arduino sketch
## Option 1: Add this repositiory as submodule. Name the submodule folder "src". 
Include header as shown below.
```
#include <Controllino.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include "src/Controllino-CMP/cmp.h"

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
```
## Option 2: Copy and place 'cmp.h' and 'cmp.cpp' in your sketch folder. 
Include header as shown below.
```
#include <Controllino.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
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
```

# CMP - ControllinoMessageProtocol

This protocol is designed to communicate with controllino devices in a command based request-response pattern. It allows you to read/write controllino IOs (Digital, Analog, Relays) and volatile memories. CMP is a simple lightweight ASCII based protocol, all characters must be ASCII.

Implemented and partly tested for Controllino Mega. Differences to other Contrllino Devices have not been taken into account. 

___
```
CMP - ControllinoMessageProtocol (V01) 	Version 'A'

	'A': V01 - 'A' like First, 'A' like ASCII, 'A' like A-Z should be enough characters for future development
	You have to replace 'V' with 'A' in any example below to make it work!

Determining CMP version of your Controllino

	You may either send an invalid command and hope for Error VEE01 or simply send Set/Read Error.
	Sending invalid commands is possibly unsafe since you may not know how an unknown CMP version may react to it. 

CMP Short Description
	Command		|	Data D			|	Description						| 	Response
	-------------------------------------------------------------------------------------------------
	VSCNNDM		|	'O','P','I'		|	Set Config						|	Error or Command (echo)
	VSDNNDM		|	'H', 'L'		|	Set Digital						|	Error or Command (echo)
	VSENNDM		|	-				|	Set Error						|	Error or Command (echo)
	VSRNNDM		|	'H', 'L'		|	Set Relay						|	Error or Command (echo)
	VSVNNDM		|	Int16 (ASCII)*	|	Set Volatile Memory Register	|	Error or Command (echo)
	VRANN		|	-				|	Read Analog Input				|	
	VRCNN		|	-				|	Read Digitals IO Configuration	|	
	VRDNN		|	-				|	Read Digitals					|	
	VRENN		|	-				|	Read Error						|	
	VRINN		|	-				|	Read Digital Inputs				|	
	VRRNN		|	-				|	Read Relays						|	Error or VRRNND(DDDD)
	VRVNN		|	-				|	Read Volatile Memory Register 	|							
	VRSNN		|	-				| 	Read System Info				| 	Error or VRSNND(DD...DD)

* Data is Mask!

CMP Message Frame: [ Version | Header | Data ]
	Version:		1 byte	[ V ]
	Header:			4 byte	[ R P NN ]	
	Data: 			N byte	[ D(D..) ]

	byte 0		V 	(Version)
					'A': Version A
	byte 1		R	(Request)
					'R': Read
					'S': Set
					'E': On Error Response
	byte 2		P	(Property)
					'A': Analog
					'C': Config
					'D': Digital
					'E': Error
					'I': Input
					'R': Relay
					'V': Volatile Memory Register
	byte 3,4 	N 	(Number)
					'00' to '99'
	byte 5		D 	(Data)
					ASCII Character
	byte 6...	M	(MASK)
					Additional Data such as Bitmasks

Request - Response Pattern:
	Controllino sends a response to any datagram received. 
	On error Controllino sends an error response.
	On SET-Requests controllino answers with a echo of the request.
		Minor differences: If Data has been set to default value the response contains the set value
	On READ-Requests Controllino answers with the command and data combined
	Responses also loop back any additional charcters that have been added to the Request
		This is simply due to how responses are generated (initially by making a copy of the request)

Note on strings:
	The current implementation can handle zero and non-zero terminated strings. 

Error are indicated by number. Known Errors:
	00: 	No Error
	01: 	CMP Version mismatch
	02: 	Digitals number out of scope
	03:		Relay number out of scope
	04:		Volatile memory access out of scope
	05:		Unknown Property
	06:		Analog number out of scope
	07:		Digital input number out of scope
	08:		Request unknown
	09:		System Info unknown
	10:		Analog Output number out of scope (Maxi Automation only)

Error Examples
	'x' equals to any invalid character
	'y' equals to any invalid or valid character
	Assuming Sizeof Volatile Memory is 50 and Contrllino Mega Configuration
	Send Command		|		Response
	-------------------------------------------------------
	x(y...)				| 		VEE01
	Vx(y...)			|		VEE08
	VSx(y...)			|		VEE05
	VRx(y...)			|		VEE05
	VSD86H(y...)		| 		VEE02
	VSC86P(y...)		|		VEE02
	VSR87H(y...)		|		VEE03
	VSV98123			|		VEE04
	VRV98(y...)			|		VEE04
	VRA98(y...)			|		VEE06
	VRI98(y...)			|		VEE07
	VRS00				|		VEE09

Valid Protocols:
VSCNNDM(MMM)	Set Config (Number) IOMODE
			NN : 	'00' to '19'				(CONTROLLINO_D0 to CONTROLLINO_D19)
					'99' Special Character		
					Applys D to all IO Configs masked by bitmask M(MMM)
			D  : 	'O' (Output)	'P' (Input_Pullup) 	'I' (Input)*
					* Every Character except 'O' or 'P' equals to Input
			Response Message: 	(echo request) on success
								VEENN	on protocol error
			Example: 		Sender to controllino:	VSC02O	(Config Digital 02 as Output)
							controllino to Sender:	VSC02O	(Config Digital 02 as Output)
			Example: 		Sender to controllino:	VSC01P	(Config Digital 01 as Input with Pullup)
							controllino to Sender:	VSC01P	(Config Digital 01 as Input with Pullup)
			Example: 		Sender to controllino:	ASC01X	(Config Digital 19 as Default: Input)
							controllino to Sender:	ASC01I	(Config Digital 19 as Input)
			Example:		Sender to controllino:	ASC99P3	(Config Digital 00 and 01 as Input with Pullup)
							controllino to Sender:	ASC99P3	(Config Digital 00 and 01 as Input with Pullup)
			Example:		Sender to controllino:	ASC99X3	(Config Digital 00 and 01 as Default: Input)
							controllino to Sender:	ASC99X3	(Config Digital 00 and 01 Input with Pullup)

VSDNNDM(MMM)	Set Digital (Number) High/Low
			NN : 	'00' to '19'		(CONTROLLINO_D0 to CONTROLLINO_D19)
					'99' Special Character
					 Applys D to all Digitals masked by bitmask M(MMM)
			D  : 	'L' (Low)	'H' (High)*
					* Every Character except 'H' equals to Low
					Setting a digital output always implies a  'SCNNO' (setting as output)
			Response Message: 	(echo request) on success
								VEENN	on protocol error
			Example: 		Sender to controllino:	ASD00H	(Set Digital 00 to High)
							controllino to Sender:	ASD00H	(Set Digital 00 to High)
			Example:		Sender to controllino:	ASD00D	(Set Digital 00 to Default: Low)
							controllino to Sender:	ASD00L	(Set Digital 00 to Low)
			Example:		Sender to controllino:	ASD99H3	(Set Digital 00 and 01 to High)
							controllino to Sender:	ASD99H3	(Set Digital 00 and 01 to High)
			Also compare to examples above

VSANND(DD)	Set Analog (Number)  - Maxi Automation Only!
			NN : 	'00' to '1'		(CONTROLLINO_AO0 to CONTROLLINO_AO1)
					'99' Special Character
					Applys D to all analog outputs
			 D 	: 	0-255 whew 0 = 0V/0mA and 255 = 10V/20mA
					See Controlino documentation for information on current output https://www.controllino.biz/tutorials/#1539865917778-d865e233-cd45
					On my Controllino Maxi Automation the 0 ohm resistors were missing.
					

VSENND			Set Error
			Set Error has no purpose
			You can use it to safely determine CMP version of Controllino or test an echo response
			Example:		Sender to controllino:	ASE
							controllino to Sender:	ASE
VSRNNDM(MMM)	Set Relay (Number) High/Low
			NN : 	'00' to '15'		(CONTROLLINO_R0 to CONTROLLINO_R15)
				 	'99' Special Character
				 	Applys D to all Relays masked by bitmask M(MMM)
			D  : 	'0' (Low)	'1' (High)*
					* Every Character except '1' equals to Low
			Response Message: 	VSRNND	(echo request) on success
								VEENN	on protocol error
			Example: 		Sender to controllino:	VSR02H	(Set Relay 02 to High)
							controllino to Sender:	VSR02H	(Set Relay 02 to High)
			Example:		Sender to controllino:	VSR00L	(Set Relay 00 to Low)
							controllino to Sender:	VSR00L	(Set Relay 00 to Low)
			Also compare to examples above

VSVNNDQ(QQQ)	Set Volatile Memory Register (Number) Value
			NN : 	'00' to '50'		(Array volmem[SIZEOFVOLMEM])
			D  : 	sign '+' or '-'
			Q(QQQ): 16bit Integer value as ASCII String
			Note: No range check of Q (no under/overflowdetection of int16_t)
			Response Message: 	VSVNND	(echo request) on success
								VEENN	on protocol error
			Example: 		Sender to controllino:	VSV00z20		(volmem[0] = 20)
							controllino to Sender:	VSV00z20		(volmem[0] = 20)
			Example: 		Sender to controllino:	VSV03z-24O		(volmem[3] = -240)
							controllino to Sender:	VSV03z-240		(volmem[3] = -240)
			Also compare to examples above

VRANN			Read Analog Input (Number)
			NN : 	'00' to '15'		(CONTROLLINO_A0 to CONTROLLINO_A15)
					'99' Special Character
				 	Reads all analog inputs in sequence
			Response Message: 	VRANND to VRANNDDDD	on success
								D(DDD) : Analog Value as numeric ASCII characters corresponding to 16 bit unsigned int
								Only in case of NN == '99'
									RANDD(DD).D(DD)....D(DD) (16 Values separated by dot)
								VEENN	on protocol error
			Example: 		Sender to controllino:	VRA00		(Read Analog Input 00)
							controllino to Sender:	VRA00824	(Read Analog Input 00, Example Data: 824)
			Example:		Sender to controllino:	VRA99		(Read all Analog Inputs)
							controllino to Sender:	ARA990.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0	(Read all Analog Inputs, Example Data: all zero)
			Also compare to examples above

VRCNN			Read Digitals IO Configuration	(CONTROLLINO_D0 to CONTROLLINO_D19)
			NN : 	'00' to '19'		(CONTROLLINO_D0 to CONTROLLINO_D19)
					'99' Special Character
			Response Message: 	VRCNNM
									M:	ASCII Characters corresponding to IO Config ('O' , 'P' , 'I')
								Only in case of NN == '99'
									VRCMMMMMMMMMMMMMMMMMMMM on success (19 times M)
								VEENN	on protocol error
			Example: 		Sender to controllino:	VRC00	(Read IO Configuration of D0)
							controllino to Sender:	VRCOI 	(Read Input, Data: configured as Input)	
			Example:		Sender to controllino:	VRC99	(Read all IO Configuration)
							controllino to Sender:	VRCOOOOOOOOOOOOOOOOOOOO	(Read Input, Data: All Inputs configured as Output)	
			Also compare to examples above

VRDNN			Read Digitals	(CONTROLLINO_D0 to CONTROLLINO_D19)
			NN : 	'00' to '19'		(CONTROLLINO_D0 to CONTROLLINO_D19)
					'99' Special Character
			Response Message: 	VRDNNM
									M:	ASCII Characters corresponding to IO State ('H', 'L')
								Only in case of NN == '99'
									VRDD.D to VRDDDDDD.DDDDD	on success
									D(DDDD).D(DDDD) : 	Bitpattern Value as ASCII String corresponding to 32 bit unsigned int (two 16 bit unsigned int)
													Highword.Lowword
										Bit Meaning: 1 = Input High ; 0 = Input Low
										MSB1,...,LSB1,MSB0,...,LSB0 
										LSB0 : 	CONTROLLINO_D0
												...
										MSB0	CONTROLLINO_D15
										LSB1	CONTROLLINO_D16
												CONTROLLINO_D17
												CONTROLLINO_D18
												CONTROLLINO_D19
												0
												...
										MSB1   	0
								VEENN	on protocol error
			Example: 		Sender to controllino:	VRD000	(Read Digital 00)
							controllino to Sender:	VRD00H	(Read Digital 00, Data: High)
			Example:		Sender to controllino:	VRD99	(Read all Digitals)
							controllino to Sender:	VRD15.65535	(Read Input, Data: All Inputs High)	
			Also compare to examples above

VRENN			Read Error
			Read Error has no purpose
			You can use it to safely determine CMP version of Controllino or test an echo response
			Example:		Sender to controllino:	ARE
							controllino to Sender:	ARE

VRINN			Read Digital Inputs	(CONTROLLINO_I16, CONTROLLINO_I17, CONTROLLINO_I18, CONTROLLINO_IN0, CONTROLLINO_IN1)
			NN : 	16, 17, 18, 0, 1
					'99' Special Character
			Response Message: 	VRINNM
									M:	ASCII Characters corresponding to IO State ('H', 'L')
								Only in case of NN == '99'
									VRINND to VRINNDDD	on success
									D(DD) : Bitpattern as ASCII String corresponding to 8 bit unsigned int
										Bit Meaning: 1 = Input High ; 0 = Input Low
										LSB : 	CONTROLLINO_IN1
												CONTROLLINO_IN0
												CONTROLLINO_I18
												CONTROLLINO_I17
												CONTROLLINO_I16
												0
										MSB		0
								VEENN	on protocol error
			Example: 		Sender to controllino:	VRI16	(Read Input 16)
							controllino to Sender:	VRI16L	(Read Input, Data: All Inputs Low)	
			Example: 		Sender to controllino:	VRI99	(Read all Inputs)
							controllino to Sender:	VRI991 	(Read Input, Data: CONTROLLINO_IN1 High, rest is Low)

VRRNN			Read Relays	(CONTROLLINO_R0 to CONTROLLINO_R15)
			NN : 	'00' to '15'		(CONTROLLINO_R0 to CONTROLLINO_R15)
					'99' Special Character
			Response Message: 	VRRNNM
									M:	ASCII Characters corresponding to IO State ('H', 'L')
								Only in case of NN == '99'
									RRD to RRDDDDD	on success
									D(DDDD) : Bitpattern as ASCII String corresponding to 16 bit unsigned int
										Bit Meaning: 1 = Input High ; 0 = Input Low
										LSB : 	CONTROLLINO_R0
												...
										MSB		CONTROLLINO_R15
								VEENN	on protocol error
			Example: 		Sender to controllino:	VRR15	(Read Relay 15)
							controllino to Sender:	VRR15L	(Read Relays 15, Data: Relays Low)	
			Example: 		Sender to controllino:	VRR99	(Read Relays)
							controllino to Sender:	VRR990	(Read Input, Data: All relays low)	

VRVNN			Read Volatile Memory Register (Number)
			NN : '00' to '50'		(Array volmem[SIZEOFVOLMEM])
			Response Message: 	VRVNNDD to VRVNNDDDDDD on success
								DD(DDDD) : 16 bit Memory register value as ASCII character string
										: first character is the sign ('+' or '-')
								VEENN	on protocol error
			Example: 			Sender to controllino:	VRV01		(Read memory element 01)					
								controllino to Sender:	VRV01+10	(Read memory element 01, Data: 10)
			Example: 			Sender to controllino:	VRV05		(Read memory element 05)					
								controllino to Sender:	VRV06-32767	(Read memory element 01, Data: -32767)
								
VRSNN			Read System Info
			NN : '01'		Return System info: "MEGA" or "MAXI_AUTO"
			Response Message: 	VRVNNMEGA or VRVNNMAXIS_AUTO on success
								VEENN	on protocol error
			Example: 			Sender to controllino:	VRS01					
								controllino to Sender:	VRS01MEGA
```
