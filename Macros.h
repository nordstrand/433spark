#ifndef NEXA_NODE_MACROS_H
#define NEXA_NODE_MACROS_H

/*
 * DEBUG should be #defined _before_ #including this file.
 */
#ifndef DEBUG
#define DEBUG 0
#endif

/* Arduino-friendly ASSERT() macro
 *
 * If DEBUG is enabled, the assertion is tested, and in case of failure,
 * a helpful error message is written to the serial port and the program
 * is aborted.
 *
 * If DEBUG is disabled, the macro resolves to nothing.
 */
#ifndef ASSERT
#if DEBUG
#define ASSERT(expr) do { \
	if (!(expr)) { \
		Serial.println(); \
		Serial.print(__FILE__); \
		Serial.print(F(":")); \
		Serial.print(__LINE__); \
		Serial.print(F(" in '")); \
		Serial.print(__PRETTY_FUNCTION__); \
		Serial.print(F("': Assertion failed: ")); \
		Serial.println(#expr); \
		Serial.flush(); \
		abort(); \
	} \
} while (0)
#else
#define ASSERT(expr, msg)
#endif
#endif

/*
 * Compute the number of elements in an array.
 */
#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(a) ((sizeof (a)) / (sizeof (a)[0]))
#endif

/*
 * The usual MIN/MAX macros
 */
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#endif
