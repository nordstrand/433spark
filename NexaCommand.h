#ifndef NEXA_NODE_NEXA_COMMAND_H
#define NEXA_NODE_NEXA_COMMAND_H

#include "Macros.h"
#include "RF433Transceiver.h"
#include "HexUtils.h"

class NexaCommand {
public: // types & constants
	enum Version {
		NEXA_INVAL, // Unknown/invalid version
		NEXA_12BIT, // Old 12-bit command format: DDDDDDDD011S
		NEXA_32BIT, // New 32-bit command format: D{24}10GSCCCC
		NEXA_END // End sentinel
	};

	// length of command string on format "V:DDDDDD:G:C:S"
	static const size_t cmd_str_len = 14;

public: // initializers
	/*
	 * Initialize NexaCommand instance from  incoming command string
	 * of the form "V:DDDDDD:G:C:S", where
	 *  - V = Nexa command version (hex digit)
	 *  - DDDDDD = 24-bit device id in hex
	 *  - G = group bit (0/1)
	 *  - C = channel in hex (0-F)
	 *  - S = state bit (0/1 == off/on)
	 */
	static bool from_cmd_str(NexaCommand & cmd, const char * buf,
	                         size_t len);

public: // queries
	// Print this Nexa command on the serial port.
	void print(Print & out) const;

	/*
	 * Transmit this Nexa command on the given RF transmitter.
	 *
	 * Return true on success, false otherwise.
	 */
	bool transmit(RF433Transceiver & rf_port) const;

private: // helpers
	void transmit_12bit(RF433Transceiver & rf_port, size_t repeats) const;
	void transmit_32bit(RF433Transceiver & rf_port, size_t repeats) const;

public: // representation
	Version version;
	byte device[3]; // 24-bit device id (NEXA_12BIT only uses last 8)
	byte channel; // 4-bit channel id (always 0 for NEXA_12BIT)
	bool group; // group bit (always false for NEXA_12BIT)
	bool state; // ON - true, OFF - false
};

bool NexaCommand::from_cmd_str(NexaCommand & cmd, const char * buf,
			       size_t len)
{
	if (len != cmd_str_len)
		return false;

	if (buf[1] != ':' || buf[8] != ':' || buf[10] != ':' ||
	    buf[12] != ':')
		return false;

	int v = Hex::parse_digit(buf[0]);
	bool d = Hex::hex2bytes(cmd.device, buf + 2, 3);
	int g = Hex::parse_digit(buf[9]);
	int c = Hex::parse_digit(buf[11]);
	int s = Hex::parse_digit(buf[13]);

	if ((v <= NEXA_INVAL && v >= NEXA_END) || !d ||
	    (g != 1 && g != 0) || c == -1 || (s != 1 && s != 0))
		return false;

	cmd.version = (Version) v;
	cmd.channel = c;
	cmd.group = g;
	cmd.state = s;
	return true;
}

void NexaCommand::print(Print & out) const
{
	const size_t device_bytes = 3;
	byte device_id[device_bytes * 2];
	Hex::bytes2hex((char *) device_id, device, device_bytes);
	out.print(version, HEX);
	out.print(':');
	out.write(device_id, ARRAY_LENGTH(device_id));
	out.print(':');
	out.print(group ? '1' : '0');
	out.print(':');
	out.print(channel, HEX);
	out.print(':');
	out.println(state ? '1' : '0');
}

bool NexaCommand::transmit(RF433Transceiver & rf_port) const
{
	switch(version) {
		case NEXA_12BIT:
			transmit_12bit(rf_port, 5);
			break;
		case NEXA_32BIT:
			transmit_32bit(rf_port, 5);
			break;
		default:
			Serial.print(F("Unknown Nexa command version "));
			Serial.println(version);
			return false;
	}

#if DEBUG
	Serial.print(F("Transmitted code: "));
	print(Serial);
	Serial.flush();
#endif

	return true;
}

void NexaCommand::transmit_12bit(RF433Transceiver & rf_port, size_t repeats) const
{
	/*
	 * SYNC: SHORT (0.35ms) HIGH, XXLONG (10.9ms) LOW
	 * '0' bit: SHORT HIGH, LONG (1.05ms) LOW, SHORT HIGH, LONG LOW
	 * '1' bit: SHORT HIGH, LONG LOW, LONG HIGH, SHORT LOW
	 */
	enum PulseLength { // in µs
		SHORT = 350,
		LONG = 3 * 350,
		XXLONG = 31 * 350,
	};

	int bits[12]; // DDDDDDDD011S
	// The 8 device bits are device[2] LSB first
	for (size_t i = 0; i < 8; ++i)
		bits[i] = device[2] >> i & 1;
	bits[8] = 0;
	bits[9] = 1;
	bits[10] = 1;
	bits[11] = state;

	for (size_t i = 0; i < repeats; ++i) {
		// SYNC
		rf_port.transmit(HIGH, SHORT);
		rf_port.transmit(LOW, XXLONG);

		// data bits
		for (size_t i = 0; i < ARRAY_LENGTH(bits); ++i) {
			if (bits[i]) { // '1'
				rf_port.transmit(HIGH, SHORT);
				rf_port.transmit(LOW, LONG);
				rf_port.transmit(HIGH, LONG);
				rf_port.transmit(LOW, SHORT);
			}
			else { // '0'
				rf_port.transmit(HIGH, SHORT);
				rf_port.transmit(LOW, LONG);
				rf_port.transmit(HIGH, SHORT);
				rf_port.transmit(LOW, LONG);
			}
		}
	}
	rf_port.transmit(HIGH, SHORT);
	rf_port.transmit(LOW);
}

void NexaCommand::transmit_32bit(RF433Transceiver & rf_port, size_t repeats) const
{
	/*
	 * SYNC: XXLONG (10.15ms) LOW, SHORT (0.31ms) HIGH,
	 *       XLONG (2.64ms) LOW, SHORT HIGH
	 * '0' bit: XSHORT (0.22ms) LOW, SHORT HIGH,
	 *          LONG (1.24ms) LOW, SHORT HIGH
	 * '1' bit: LONG LOW, SHORT HIGH,
	 *          XSHORT LOW, SHORT HIGH
	 */
	enum PulseLength { // in µs
		XSHORT = 215,
		SHORT = 310,
		LONG = 1236,
		XLONG = 2643,
		XXLONG = 10150,
	};

	int bits[32]; // DDDDDDDDDDDDDDDDDDDDDDDD10GSCCCC
	// The 24 device bits are device[2..0] LSB first
	for (size_t i = 0; i < 24; ++i)
		bits[i] = device[2 - i / 8] >> (i % 8) & 1;
	bits[24] = 1;
	bits[25] = 0;
	bits[26] = group;
	bits[27] = state;
	bits[28] = channel & B1000;
	bits[29] = channel & B100;
	bits[30] = channel & B10;
	bits[31] = channel & B1;

	for (size_t i = 0; i < repeats; ++i) {
		// SYNC
		rf_port.transmit(LOW, XXLONG);
		rf_port.transmit(HIGH, SHORT);
		rf_port.transmit(LOW, XLONG);
		rf_port.transmit(HIGH, SHORT);

		// data bits
		for (size_t i = 0; i < ARRAY_LENGTH(bits); ++i) {
			if (bits[i]) { // '1'
				rf_port.transmit(LOW, LONG);
				rf_port.transmit(HIGH, SHORT);
				rf_port.transmit(LOW, XSHORT);
				rf_port.transmit(HIGH, SHORT);
			}
			else { // '0'
				rf_port.transmit(LOW, XSHORT);
				rf_port.transmit(HIGH, SHORT);
				rf_port.transmit(LOW, LONG);
				rf_port.transmit(HIGH, SHORT);
			}
		}
	}
	rf_port.transmit(LOW);
}

#endif
