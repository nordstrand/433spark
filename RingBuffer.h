#ifndef NEXA_NODE_RING_BUFFER_H
#define NEXA_NODE_RING_BUFFER_H

#include "Macros.h"

/**
 * Simple ring buffer with ISR-safe write side.
 *
 * This is a simple and straightforward ring buffer that is suitable for
 * forwarding values from an ISR to the main loop.
 *
 * There is no guarantee that pushing elements onto the buffer won't
 * overwrite the read-side of the ring buffer. The only way to prevent
 * buffer overflow is to make sure that the buffer is sufficiently large,
 * and that the read-side is serviced often enough.
 */
template<typename T>
class RingBuffer {
public: // administrivia
	RingBuffer(size_t size)
		: buffer(new T[size]), size(size), r_pos(0), w_pos(0) { }

	~RingBuffer() { delete[] buffer; }

	void print(Print & out) const
	{
		out.print(F("<RingBuffer, size = "));
		out.print(size);
		out.print(F(", r_pos = "));
		out.print(r_pos);
		out.print(F(", w_pos = "));
		out.print(w_pos);
		out.print(F(", top = "));
		out.print(r_top());
		out.println(F(">"));
	}

public: // read-side queries
	/// Return true iff there is no available data to be read.
	bool r_empty() const { return w_pos == r_pos; }

	/**
	 * Return the first readable element.
	 *
	 * This method may only be called if there are one or more
	 * elements available.
	 */
	T r_top() const
	{
		ASSERT(!r_empty());
		return buffer[r_pos];
	}

	/**
	 * Return pointer to readable data.
	 *
	 * The number of readable elements starting from the returned
	 * pointer is given by r_buf_len(), which might be less than
	 * r_available() (because of wrap-around).
	 *
	 * This method does not consume any elements from the ring buffer;
	 * use r_consume() for that.
	 */
	const T * r_buf() const { return buffer + r_pos; }

	/**
	 * Return pointer to the wrapped/trailing portion of the buffer.
	 *
	 * This returns a valid pointer even when the ring buffer is NOT
	 * wrapped, but in this case, r_wrapped_buf_len() will return 0,
	 * hence nothing should be read from the returned pointer
	 */
	const T * r_wrapped_buf() const { return buffer; }

	/**
	 * Return the number of elements available from r_buf().
	 *
	 * This may be less than the total number of available elements
	 * (because of ring buffer wrap-around).
	 */
	size_t r_buf_len() const
	{
		size_t i_w = w_pos; // cache volatile access
		return (i_w >= r_pos ? i_w : size) - r_pos;
	}

	/**
	 * Return the number of elements available from r_wrapped_buf().
	 *
	 * This will only be non-zero when the buffer is wrapped, i.e.
	 * when the write pointer has wrapped back to the beginning of the
	 * buffer, while the read pointer has not. At any time,
	 * r_available() == r_buf_len() + r_wrapped_buf_len()
	 */
	size_t r_wrapped_buf_len() const
	{
		size_t i_w = w_pos; // cache volatile access
		return i_w >= r_pos ? 0 : i_w;
	}

	/**
	 * Return the total number of available elements.
	 *
	 * This may be less than the total number of available elements
	 * (because of ring buffer wrap-around).
	 */
	size_t r_available() const
	{
		size_t i_w = w_pos; // cache volatile access
		return (i_w < r_pos ? i_w + size : i_w) - r_pos;
	}

public: // read-side commands
	/**
	 * Consume and return the first readable element.
	 *
	 * This method may only be called if there are one or more
	 * elements available.
	 */
	T r_pop()
	{
		ASSERT(!r_empty());
		size_t i = r_pos;
		r_consume(1);
		return buffer[i];
	}

	/**
	 * Consume the given number of bytes.
	 *
	 * The given length MUST be <= r_available().
	 */
	void r_consume(size_t len)
	{
		ASSERT(len <= r_available());
		r_pos = (r_pos + len) % size;
	}

public: // write-side commands
	/**
	 * Push another element onto the ring buffer.
	 */
	void w_push(T element)
	{
		buffer[w_pos++] = element;
		w_pos %= size;
	}

private: // representation
	T * buffer;
	const size_t size;
	size_t r_pos;
	volatile size_t w_pos;
};

#endif
