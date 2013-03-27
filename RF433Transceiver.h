#ifndef NEXA_NODE_RF433_TRANSCEIVER_H
#define NEXA_NODE_RF433_TRANSCEIVER_H

#include "FastPort.h"

/*
 * Encapsulate a 433MHz receiver/transmitter pair (like the WLS107B4B
 * documented at [1]), where both the receiver (RX) and transmitter (TX)
 * are hooked up to the same JeeNode Port, like this:
 *  - JeeNode Port P (or +): VCC on TX
 *  - JeeNode Port +:        VCC on RX
 *  - JeeNode Port G:        GND on both TX and RX
 *  - JeeNode Port D:        Data In on TX
 *  - JeeNode Port A:        Data Out on RX
 *
 * The (now-discontinued) OOK 433 Plug from JeeLabs [2] conforms to this.
 *
 * [1]: http://www.seeedstudio.com/wiki/index.php?title=433Mhz_RF_link_kit
 * [2]: http://jeelabs.org/oo1
 */
class RF433Transceiver {
public:
	RF433Transceiver(FastPort port) : port(port)
	{
		port.d_mode(OUTPUT);
		port.a_mode(INPUT);
	}

	/*
	 * Transmit the given HIGH/LOW pulse for the given time.
	 *
	 * The HIGH/LOW state is set on the TX pin, and then this method
	 * busy-waits until the given time has elapsed. In order to end
	 * the pulse, the caller must immediately set the opposite pulse.
	 *
	 * The timing seems to be within 1% accuracy for repeated
	 * invocations for pulse lengths between 0.1ms and 16ms.
	 */
	inline void transmit(byte pulse, unsigned short usecs = 0)
	{
		port.d_write(pulse);

		if (usecs <= 1)
			return;
		else
			usecs -= 2; // discount overhead in this function

		if (usecs)
			delayMicroseconds(usecs);
	}

	// Return current RX state (true iff 433MHz carrier present)
	inline bool rx_pin() { return port.a_read(); }

private:
	FastPort port;
};

#endif
