#ifndef NEXA_NODE_HEX_UTILS_H
#define NEXA_NODE_HEX_UTILS_H

//include <Arduino.h>

namespace Hex {
	// return 0 - 15 for given char '0' - 'F'/'f'; otherwise return -1
	int parse_digit(char c)
	{
		switch(c) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				return c - '0';
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
				return 10 + c - 'a';
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				return 10 + c - 'A';
			default:
				return -1;
		}
	}

	// convert 2 hex digits into 0 - 255; return -1 on failure
	int parse_byte(char h, char l)
	{
		int ih = parse_digit(h);
		int il = parse_digit(l);
		if (ih == -1 || il == -1)
			return -1;
		return ih << 4 | il;
	}

	/*
	 * Convert hex string into corresponding byte string.
	 *
	 * The "len" * 2 first hex digits in "src" are parsed and their
	 * byte values stored into the first "len" bytes of "dst".
	 *
	 * Return true on success, or false if any non-hex digits are
	 * found (in which case the contents of target is undefined).
	 */
	bool hex2bytes(byte * dst, const char * src, size_t len)
	{
		for (size_t i = 0; i < len; ++i) {
			int b = parse_byte(src[i * 2], src[i * 2 + 1]);
			if (b == -1)
				return false;
			dst[i] = b;
		}
		return true;
	}

	/*
	 * Convert byte string into corresponding hex string.
	 *
	 * The first "len" bytes of "src" and converted into hex, and
	 * written into "dst", which must have room for at least "len" * 2
	 * characters.
	 */
	void bytes2hex(char * dst, const byte * src, size_t len)
	{
		static const char hex[] = "0123456789ABCDEF";
		for (size_t i = 0; i < len; ++i) {
			dst[i * 2] = hex[src[i] >> 4 & 0b1111];
			dst[i * 2 + 1] = hex[src[i] & 0b1111];
		}
	}
}

#endif
