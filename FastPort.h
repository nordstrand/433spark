#ifndef NEXA_NODE_FAST_PORT_H
#define NEXA_NODE_FAST_PORT_H


#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

#define abs(x) ((x)>0?(x):-(x))
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

typedef uint16_t word;

#define bit(b) (1UL << (b))


#define TX_PIN D3
#define RX_PIN D4

//include "Arduino.h"

/*
 * A faster replacement for JeeLib's Port class
 *
 * Based on the XPort class that was described in [1], even though that
 * code never seems to have made it into JeeLib...
 *
 * [1]: http://jeelabs.org/2010/01/12/c-templates/
 *
 * Uses direct register access instead of Arduino's digitalRead,
 * analogWrite(), etc. functions.
 *
 * The template argument N is the Port number (1 - 4 on a regular JeeNode)
 */
class FastPort {
private:
	byte portNum;

public:
	FastPort(byte portNum) : portNum(portNum) { }

	/// Digital pin

	inline void d_mode(byte dir)
	{
        pinMode(TX_PIN, OUTPUT);
        //bitWrite(DDRD, 3 + portNum, dir == OUTPUT ? 1 : 0);
		//if (dir != OUTPUT)
		//	bitWrite(PORTD, 3 + portNum, dir == INPUT_PULLUP);
	}

	//inline byte d_read() { return bitRead(PIND, 3 + portNum); }
	inline void d_write(byte v) { digitalWrite(TX_PIN, v); }

	/// Analog pin

	inline void a_mode(byte dir)
	{
		pinMode(RX_PIN, INPUT);
        //bitWrite(DDRC, portNum - 1, dir == OUTPUT ? 1 : 0);
		//if (dir != OUTPUT)
	    //bitWrite(PORTC, portNum - 1, dir == INPUT_PULLUP);
	}

	inline bool a_read() { 
        return digitalRead(RX_PIN);
       // Serial.print("REad ");
        //Serial.println(b);
        //return b;
    }
	//inline void a_write(byte v) { bitWrite(PORTC, portNum - 1, v); }
};

#endif
