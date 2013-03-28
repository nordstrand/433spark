#ifndef NEXA_NODE_PULSE_PARSER_H
#define NEXA_NODE_PULSE_PARSER_H

#include "RingBuffer.h"

#include <Arduino.h>

/*
 * Process incoming pulses from the RX module, and generate bit sequences.
 *
 * Nexa RF signals come in a couple of different formats, but they
 * generally consist of a SYNC waveform followed by a series of waveforms
 * representing data bits.
 *
 * This class processes incoming pulses - as produced by for example
 * RF433Transceiver::rx_get_pulse() - and generates pseudo-bit sequences
 * consisting of letters ('A', 'B') representing various SYNC pulses,
 * followed by strings of '1's and '0's representing data bits.
 *
 * The generated strings are pushed onto a RingBuffer, enabling the parser
 * to be run from an ISR.
 */
class PulseParser {
public:
	PulseParser(RingBuffer<char> & buffer)
		: buffer(buffer), cur_state(UNKNOWN), cur_bit(0) { }

	/// drive state machine with pulses from Nexa RF waveform
	bool operator()(int pulse);

private: // helpers
	/// classify pulses by length into category 1..5
	static int quantize_pulse(int p);

private: // representation
	RingBuffer<char> & buffer;
	enum State {
		UNKNOWN, SX1, SX2, SX3,
		DA0, DA1, DA2, DA3,
		DB0, DB1, DB2, DB3,
	} cur_state;
	byte cur_bit;
};

/*
 * Classify the given pulse length into the following categories:
 *
 * 0: Invalid pulse length (i.e. not within any of the below categories)
 * 1:    0µs <= p <   512µs
 * 2:  512µs <= p <  2048µs
 * 3: 2048µs <= p <  4096µs
 * 4: 4096µs <= p <  8192µs
 * 5: 8192µs <= p < 16384µs
 *
 * The above categories (1..5) are positive (1..5) for HIGH pulses, and
 * negative (-1..-5) for LOW pulses.
 */
int PulseParser::quantize_pulse(int p)
{
	ASSERT(p >= -INT_MAX);
	int sign = (p > 0) ? 1 : -1;
	p = (p * sign) >> 9; // Divide pulse length (abs value) by 512
	// there are 7 bits left in p representing the pulse length as a
	// multiple of 512µs; map those into the above categories:
	ASSERT(p <= B01111111);
	static const uint8_t m[128] = {
		1, // 0µs <= p < 512µs
		2, 2, 2, // 512µs <= p < 2048µs
		3, 3, 3, 3, // 2048µs <= p < 4096µs
		4, 4, 4, 4, 4, 4, 4, 4, // 4096µs <= p < 8192µs
		5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, // 8192µs <= p < 16384µs
		// auto-initialized to 0 // 16384µs <= p
	};
	return m[p] * sign;
}

/*
 * Drive the state machine forward with the given pulse.
 *
 * The sign of the given pulse is its state (positive = HIGH, negative =
 * LOW), and the magnitude is its length in µs.
 *
 * This method pushes pseudo-bits onto the ring buffer as they become
 * apparent from the given pulses. The caller is responsible for consuming
 * and decoding the bit sequences in the ring buffer.
 *
 * Return true if we're currently in a state indicating that no Nexa
 * command is currently being received. Otherwise, return false if we're
 * "busy" receiving what might end up being a valid command.
 */
bool PulseParser::operator()(int pulse)
{
	State new_state = UNKNOWN;
	int p = quantize_pulse(pulse); // current pulse
	switch (p) {
		case -5: // LOW: 8192µs <= pulse < 16384µs => SYNC start
			new_state = SX1;
			break;
		case -3: // LOW: 2048µs <= pulse < 4096µs
			if (cur_state == SX2) // cmd format A
				new_state = SX3;
			break;
		case -2: // LOW: 512µs <= pulse < 2048µs
			if (cur_state == DA0) {
				cur_bit = '1';
				new_state = DA1;
			}
			else if (cur_state == DA2 && cur_bit == '0')
				new_state = DA3;
			else if (cur_state == SX2 || cur_state == DB1) {
				if (cur_state == SX2) // cmd format B
					buffer.w_push('B');
				new_state = DB2;
			}
			else if (cur_state == DB3 && cur_bit == '0') {
				buffer.w_push(cur_bit);
				cur_bit = 0;
				new_state = DB0;
			}
			break;
		case -1: // LOW: 0µs <= pulse < 512µs
			if (cur_state == DA0) {
				cur_bit = '0';
				new_state = DA1;
			}
			else if (cur_state == DA2 && cur_bit == '1')
				new_state = DA3;
			else if (cur_state == DB3 && cur_bit == '1') {
				buffer.w_push(cur_bit);
				cur_bit = 0;
				new_state = DB0;
			}
			break;
		case 2: // HIGH: 512µs <= pulse < 2048µs
			if (cur_state == DB2) {
				cur_bit = '1';
				new_state = DB3;
			}
			break;
		case 1: // HIGH: 0µs <= pulse < 512µs
			switch (cur_state) {
				case SX1:
					new_state = SX2;
					break;
				case SX3:
					buffer.w_push('A');
					new_state = DA0;
					break;
				case DA1:
					new_state = DA2;
					break;
				case DA3:
					buffer.w_push(cur_bit);
					cur_bit = 0;
					new_state = DA0;
					break;
				case DB0:
					new_state = DB1;
					break;
				case DB2:
					cur_bit = '0';
					new_state = DB3;
					break;
			}
			break;
	}
	cur_state = new_state;
	return cur_state == UNKNOWN;
}

#endif
