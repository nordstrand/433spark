#ifndef NEXA_NODE_RF433_TRANSCEIVER_H
#define NEXA_NODE_RF433_TRANSCEIVER_H

#include "Macros.h"
#include "FastPort.h"

#include <limits.h>

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
	RF433Transceiver(FastPort port)
		: port(port), pulse_start(0), pulse_state(false)
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

	/*
	 * Block and return the current pulse from the RX.
	 *
	 * This will block until rx_pin() changes. At that point it will
	 * return an int whose absolute value is the pulse length in µs,
	 * and the sign is positive for a HIGH pulse and negative for a
	 * LOW pulse.
	 *
	 * This method must be called with high frequency, at least twice
	 * as high as the frequency of the shortest pulse to be detected.
	 *
	 * This method assumes that the longest pulse of interest is <
	 * INT_MAX µs. The length of the returned pulse is pinned to
	 * INT_MAX/-INT_MAX (for a HIGH/LOW pulses, respectively).
	 */
	int rx_get_pulse()
	{
		while (pulse_state == rx_pin())
			; // spin until state changes
		unsigned long now = micros();
		bool ret_state = pulse_state;
		pulse_state = rx_pin();

		unsigned long elapsed = now - pulse_start;
		pulse_start = now;
		return int(MIN(elapsed, INT_MAX)) * (ret_state ? 1 : -1);
	}

private:
	FastPort port;
	unsigned long pulse_start;
	bool pulse_state;
};

#endif
