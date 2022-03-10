#include "RingBuffer.h"

#include <memory>
#include <iostream>
#include <WinSock2.h>

namespace pkodev
{
	// Constructor
	RingBuffer::RingBuffer(std::size_t size) :
		m_size(size + 1),
		m_write_pos(0),
		m_read_pos(0),
		m_data_pos(0),
		m_buffer(new char[size + 1]{ 0x00 })
	{

	}

	// Destructor
	RingBuffer::~RingBuffer()
	{
		// Free memory
		if (m_buffer != nullptr)
		{
			delete[] m_buffer;
		}
	}

	// Get the number of bytes available for writing to the buffer
	std::size_t RingBuffer::get_writeable_length() const
	{
		// Check that position of beginning of data is after position of writing or coincides with it
		if (m_write_pos >= m_data_pos)
		{
			// Bytes can be written in two write operations
			return (m_size - m_write_pos + m_data_pos - 1);
		}

		// Bytes can be written in one write operation
		return (m_data_pos - m_write_pos - 1);
	}

	// Get the number of bytes available for reading from the buffer
	std::size_t RingBuffer::get_readable_length() const
	{
		// Check that position of beginning of data is after position of reading or coincides with it
		if (m_write_pos < m_read_pos)
		{
			// Bytes can be read in two read operations
			return (m_size - m_read_pos + m_write_pos);
		}

		// Bytes can be read in one read operation
		return (m_write_pos - m_read_pos);
	}

	// Check if 'n' bytes can be written to the buffer
	bool RingBuffer::can_write_bytes(std::size_t n) const
	{
		return (n <= get_writeable_length());
	}

	// Check if 'n' bytes can be read from the buffer
	bool RingBuffer::can_read_bytes(std::size_t n) const
	{
		return (n <= get_readable_length());
	}

	// Get the number of bytes written to the buffer
	std::size_t RingBuffer::get_used_length() const
	{
		return (m_size - get_writeable_length());
	}

	// Write a block of data to the buffer
	void RingBuffer::write(const char* data, std::size_t length)
	{
		// Check that there are 'length' bytes available for writing in the buffer
		if (get_writeable_length() < length)
		{
			throw ring_buffer_exception(
				"RingBuffer::write(): Not enough free space in the buffer for writing operation!"
			);
		}

		// The number of bytes that can be written at a time
		std::size_t till_end = m_size - m_write_pos;

		// Check that we can write a block in one single writing operation
		if (till_end < length)
		{
			// Copy part of the block to the buffer
			std::memcpy(reinterpret_cast<void *>(m_buffer + m_write_pos), reinterpret_cast<const void *>(data), till_end);

			// Reset writing position
			m_write_pos = 0;

			// Increase the pointer to the data block
			data += till_end;

			// Reduce the number of bytes left to write
			length -= till_end;
		}

		// Copy the rest of the block to the buffer
		std::memcpy(reinterpret_cast<void*>(m_buffer + m_write_pos), reinterpret_cast<const void*>(data), length);

		// Increase writing position
		m_write_pos += length;
	}

	// Write operation for type ubyte_t
	void RingBuffer::write_ubyte(ubyte_t ubyte)
	{
		write(reinterpret_cast<const char*>(&ubyte), sizeof(ubyte));
	}

	// Write operation for type uint8_t
	void RingBuffer::write_uint8(uint8_t uint8)
	{
		write(reinterpret_cast<const char*>(&uint8), sizeof(uint8));
	}

	// Write operation for type uint16_t
	void RingBuffer::write_uint16(uint16_t uint16)
	{
		uint16 = htons(static_cast<u_short>(uint16));
		write(reinterpret_cast<const char*>(&uint16), sizeof(uint16));
	}

	// Write operation for type uint32_t
	void RingBuffer::write_uint32(uint32_t uint32)
	{
		uint32 = htonl(static_cast<u_long>(uint32));
		write(reinterpret_cast<const char*>(&uint32), sizeof(uint32));
	}

	// Write operation for type string_t
	void RingBuffer::write_string(const string_t& string)
	{
		// Calculate the length of the string, taking into account the null character
		std::size_t length = string.length() + 1;

		// Check the length
		if (length > 512)
		{
			// Throw an exception
			throw ring_buffer_exception("RingBuffer::write_string(): Incorrect string length!");
		}
		
		// Write the string length
		write_uint16(static_cast<uint16_t>(length));

		// Write the string
		write(string.c_str(), length);
	}

	// Read a block of data from the buffer
	void RingBuffer::read(char* dst, std::size_t length)
	{
		// Check that there are 'length' bytes available for reading in the buffer
		if (get_readable_length() < length)
		{
			throw ring_buffer_exception(
				"RingBuffer::read(): Not enough available bytes in the buffer for reading operation!"
			);
		}

		// The number of bytes that can be read at a time
		std::size_t till_end = m_size - m_read_pos;

		// Check that we can read a block in one single reading operation
		if (till_end < length)
		{
			// Copy part of the block to the destination buffer
			std::memcpy(reinterpret_cast<void*>(dst), reinterpret_cast<const void*>(m_buffer + m_read_pos), till_end);

			// Reset reading position
			m_read_pos = 0;

			// Increase the pointer to the destination buffer
			dst += till_end;

			// Reduce the number of bytes left to read
			length -= till_end;
		}

		// Copy the rest of the block to the destination buffer
		std::memcpy(reinterpret_cast<void*>(dst), reinterpret_cast<const void*>(m_buffer + m_read_pos), length);

		// Increase reading position
		m_read_pos += length;
	}

	// Read operation for type ubyte_t
	ubyte_t RingBuffer::read_ubyte()
	{
		ubyte_t ubyte = 0;
		read(reinterpret_cast<char *>(&ubyte), sizeof(ubyte));

		return ubyte;
	}

	// Read operation for type uint8_t
	uint8_t RingBuffer::read_uint8()
	{
		ubyte_t uint8 = 0;
		read(reinterpret_cast<char*>(&uint8), sizeof(uint8));

		return uint8;
	}

	// Read operation for type uint16_t
	uint16_t RingBuffer::read_uint16()
	{
		uint16_t uint16 = 0;
		read(reinterpret_cast<char*>(&uint16), sizeof(uint16));

		return static_cast<uint16_t>(ntohs(static_cast<u_short>(uint16)));
	}

	// Read operation for type uint32_t
	uint32_t RingBuffer::read_uint32()
	{
		uint32_t uint32 = 0;
		read(reinterpret_cast<char*>(&uint32), sizeof(uint32));

		return static_cast<uint32_t>(ntohl(static_cast<u_long>(uint32)));
	}

	// Read operation for type string_t
	string_t RingBuffer::read_string()
	{
		// Read the length of the string
		std::size_t length = static_cast<std::size_t>(read_uint16());

		// Check the length
		if (length < 1 || length > 512)
		{
			// Throw an exception
			throw ring_buffer_exception("RingBuffer::read_string(): Incorrect string length!");
		}

		// Check that the buffer has the required number of bytes
		if (get_readable_length() < length)
		{
			// Throw an exception
			throw ring_buffer_exception(
				"RingBuffer::read_string(): Not enough space in the buffer for reading operation!"
			);
		}

		// Create a string
		string_t string{ "" };

		// Allocate memory for the string
		string.reserve(length);
	
		// The number of bytes that can be read at a time
		std::size_t till_end = m_size - m_read_pos;

		// Check that we can read a string in one single reading operation
		if (till_end < length)
		{
			// Copy the bytes from the buffer to the string
			string.assign(m_buffer + m_read_pos, till_end);

			// Reset reading position
			m_read_pos = 0;

			// Reduce the number of bytes left to read
			length -= till_end;
		}

		// Copy the bytes from the buffer to the string
		string.append(m_buffer + m_read_pos, length);

		// Remove the null byte
		string.pop_back();

		// Increase reading position
		m_read_pos += length;

		// Return the string
		return string;
	}

	// Remove bytes that have been read from the buffer
	void RingBuffer::read_commit()
	{
		m_data_pos = m_read_pos;
	}

	// Restart next read operation
	void RingBuffer::read_rollback()
	{
		m_read_pos = m_data_pos;
	}

	// Skip reading of the specified number of bytes
	void RingBuffer::read_skip(std::size_t n)
	{
		// Check that there are 'n' bytes available for reading in the buffer
		if (can_read_bytes(n) == false)
		{
			throw ring_buffer_exception(
				"RingBuffer::read_skip(): Not enough available bytes in the buffer for read skipping operation!"
			);
		}

		// Increase the read position
		m_read_pos += n;

		// Check that we have gone beyond the end of the buffer
		if (m_read_pos > m_size)
		{
			// Subtract the size of the buffer
			m_read_pos -= m_size;
		}
	}

	// Clear buffer
	void RingBuffer::clear()
	{
		// Reset positions
		m_read_pos = m_write_pos = m_data_pos = 0;
	}
}