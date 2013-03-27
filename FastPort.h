#ifndef NEXA_NODE_FAST_PORT_H
#define NEXA_NODE_FAST_PORT_H

#include <Arduino.h>

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
		bitWrite(DDRD, 3 + portNum, dir == OUTPUT ? 1 : 0);
		if (dir != OUTPUT)
			bitWrite(PORTD, 3 + portNum, dir == INPUT_PULLUP);
	}

	inline byte d_read() { return bitRead(PIND, 3 + portNum); }
	inline void d_write(byte v) { bitWrite(PORTD, 3 + portNum, v); }

	/// Analog pin

	inline void a_mode(byte dir)
	{
		bitWrite(DDRC, portNum - 1, dir == OUTPUT ? 1 : 0);
		if (dir != OUTPUT)
			bitWrite(PORTC, portNum - 1, dir == INPUT_PULLUP);
	}

	inline byte a_read() { return bitRead(PINC, portNum - 1); }
	inline void a_write(byte v) { bitWrite(PORTC, portNum - 1, v); }
};

#endif
