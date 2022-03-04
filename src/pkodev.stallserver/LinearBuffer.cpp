#include "LinearBuffer.h"

#include <iostream>
#include <iomanip>
#include <WinSock2.h>

namespace pkodev
{
	// Constructor
	LinearBuffer::LinearBuffer(std::size_t size) :
		m_size(size),
		m_write_pos(0),
		m_read_pos(0),
		m_buffer(new char[size] {0x00})
	{

	}

	// Destructor
	LinearBuffer::~LinearBuffer()
	{
		// Free memory
		if (m_buffer != nullptr)
		{
			delete[] m_buffer;
		}
	}

	// Write a block of data to the buffer
	void LinearBuffer::write(const char* data, std::size_t length)
	{
		// Check the write position
		if (can_write_bytes(length) == false)
		{
			throw linear_buffer_exception("LinearBuffer::write(): Not enough space for writing operation!");
		}

		// Write data to the buffer
		std::memcpy(reinterpret_cast<void*>(m_buffer + m_write_pos), reinterpret_cast<const void*>(data), length);

		// Increase the write position
		m_write_pos += length;
	}

	// Write operation for type ubyte_t
	void LinearBuffer::write_ubyte(ubyte_t ubyte)
	{
		write(reinterpret_cast<const char*>(&ubyte), sizeof(ubyte));
	}

	// Write operation for type uint8_t
	void LinearBuffer::write_uint8(uint8_t uint8)
	{
		write(reinterpret_cast<const char*>(&uint8), sizeof(uint8));
	}
	
	// Write operation for type uint16_t
	void LinearBuffer::write_uint16(uint16_t uint16)
	{
		uint16 = htons(uint16);
		write(reinterpret_cast<const char*>(&uint16), sizeof(uint16));
	}

	// Write operation for type uint32_t
	void LinearBuffer::write_uint32(uint32_t uint32)
	{
		uint32 = htonl(uint32);
		write(reinterpret_cast<const char*>(&uint32), sizeof(uint32));
	}

	// Write operation for type string_t
	void LinearBuffer::write_string(const string_t& string)
	{
		// Calculate the length of the string, taking into account the null character
		std::size_t length = string.length() + 1;

		// Write the string as an array of bytes
		write_bytearray(
			reinterpret_cast<const ubyte_t *>(string.c_str()), 
			static_cast<uint16_t>(string.length() + 1)
		);
	}

	// Write operation for byte array
	void LinearBuffer::write_bytearray(const ubyte_t* data, uint16_t size)
	{
		// Write the length of byte array
		write_uint16(size);

		// Write the byte array
		write(reinterpret_cast<const char*>(data), static_cast<std::size_t>(size));
	}

	// Read a block of data from the buffer
	void LinearBuffer::read(char* dst, std::size_t length)
	{
		// Check the read position
		if (can_read_bytes(length) == false)
		{
			throw linear_buffer_exception("LinearBuffer::read(): Not enough space for reading operation!");
		}

		// Write data to the destination buffer
		std::memcpy(reinterpret_cast<void*>(dst), reinterpret_cast<const void*>(m_buffer + m_read_pos), length);

		// Increase the read position
		m_read_pos += length;
	}

	// Read operation for type ubyte_t
	ubyte_t LinearBuffer::read_ubyte()
	{
		ubyte_t ubyte = 0;
		read(reinterpret_cast<char*>(&ubyte), sizeof(ubyte));

		return ubyte;
	}

	// Read operation for type uint8_t
	uint8_t LinearBuffer::read_uint8()
	{
		uint8_t uint8 = 0;
		read(reinterpret_cast<char*>(&uint8), sizeof(uint8));

		return uint8;
	}

	// Read operation for type uint16_t
	uint16_t LinearBuffer::read_uint16()
	{
		uint16_t uint16 = 0;
		read(reinterpret_cast<char*>(&uint16), sizeof(uint16));

		return ntohs(uint16);
	}

	// Read operation for type uint32_t
	uint32_t LinearBuffer::read_uint32()
	{
		uint32_t uint32 = 0;
		read(reinterpret_cast<char*>(&uint32), sizeof(uint32));

		return ntohl(uint32);
	}

	// Read operation for type string_t
	string_t LinearBuffer::read_string()
	{
		// Read the length of the string
		uint16_t length = read_uint16();

		// Check that the buffer has the required number of bytes
		if (can_read_bytes(static_cast<std::size_t>(length)) == false)
		{
			throw linear_buffer_exception("LinearBuffer::read_string(): Not enough space in the buffer for reading operation!");
		}

		// Create a string
		string_t string("");

		// Allocate memory for the string
		string.reserve(length);

		// Copy the bytes from the buffer to the string
		string.append(m_buffer + m_read_pos, length);

		// Remove the null byte
		string.pop_back();

		// Increase the read position
		m_read_pos += length;

		// Return the string
		return string;
	}

	// Read operation for byte array
	uint16_t LinearBuffer::read_bytearray(ubyte_t* dst, std::size_t size)
	{
		// Read the size of the array
		uint16_t length = read_uint16();

		// Check that the size of the array does not exceed the size of the buffer
		if (length <= size)
		{
			// Read the array of bytes
			read(reinterpret_cast<char*>(dst), static_cast<std::size_t>(length));
		}
		else
		{
			// Roll back the read operation of the array size
			m_read_pos -= sizeof(length);

			// Throw an exception
			throw linear_buffer_exception("LinearBuffer::read_bytearray(): Destination buffer is smaller than byte array!");
		}

		// Return the read array size
		return length;
	}

	// Get byte by index
	ubyte_t LinearBuffer::byte(std::size_t index) const
	{
		// Check the index
		if (index >= m_size)
		{
			// Incorrect index
			throw linear_buffer_exception("LinearBuffer::byte(): Index is out of bounds!");
		}

		// Return the byte by index
		return static_cast<ubyte_t>(m_buffer[index]);
	}

	// Set read position
	void LinearBuffer::seek_read(std::size_t index, seek_type type)
	{
		// Check seek mode
		switch (type)
		{
			// Relative to the beginning
			case seek_type::begin:
				{
					// Check the new position
					if (index < m_size)
					{
						// Set the new read position
						m_read_pos = index;
						return;
					}
				}
				break;

			// Relative to the current position
			case seek_type::current:
				{
					// Check the new position
					if (can_read_bytes(index) == true)
					{
						// Set the new read position
						m_read_pos += index;
						return;
					}
				}
				break;

			// Relative to the ending
			case seek_type::end:
				{
					// Check the new position
					if (index <= m_size)
					{
						// Set the new read position
						m_read_pos = (m_size - index);
						return;
					}
				}
				break;
		}

		// Throw an exception
		throw linear_buffer_exception("LinearBuffer::seek_read(): Index is out of bounds!");
	}

	// Set write position
	void LinearBuffer::seek_write(std::size_t index, seek_type type)
	{
		// Check seek mode
		switch (type)
		{
			// Relative to the beginning
			case seek_type::begin:
				{
					// Check the new position
					if (index < m_size)
					{
						// Set the new write position
						m_write_pos = index;
						return;
					}
				}
				break;

			// Relative to the current position
			case seek_type::current:
				{
					// Check the new position
					if (can_write_bytes(index) == false)
					{
						// Set the new write position
						m_write_pos += index;
						return;
					}
				}
				break;

			// Relative to the ending
			case seek_type::end:
				{
					// Check the new position
					if (index <= m_size)
					{
						// Set the new write position
						m_write_pos = (m_size - index);
						return;
					}
				}
				break;
		}

		// Throw an exception
		throw linear_buffer_exception("LinearBuffer::seek_write(): Index is out of bounds!");
	}

	// Check that there are 'length' bytes in the buffer to write
	bool LinearBuffer::can_write_bytes(std::size_t length) const
	{
		return ((m_write_pos + length) <= m_size);
	}

	// Check that there are 'length' bytes in the buffer to read
	bool LinearBuffer::can_read_bytes(std::size_t length) const
	{
		return ((m_read_pos + length) <= m_size);
	}

	// Clear buffer
	void LinearBuffer::clear()
	{
		// Reset the read and write positions
		m_write_pos = 0;
		m_read_pos = 0;

		// Fill the buffer with zeros
		std::memset(reinterpret_cast<void*>(m_buffer), 0x00, m_size);
	}

	// Apply a custom function to data in the buffer
	void LinearBuffer::apply(std::size_t begin, std::size_t end, user_cb_t func)
	{
		// Check the starting index
		if (begin >= m_size)
		{
			throw linear_buffer_exception("LinearBuffer::apply(): Begin index is equal or greater than buffer size!");
		}

		// Check the ending index
		if (end > m_size)
		{
			throw linear_buffer_exception("LinearBuffer::apply(): End index is greater than buffer size!");
		}

		// Check that the starting index is less than the ending one
		if (begin >= end)
		{
			throw linear_buffer_exception("LinearBuffer::apply(): Begin index is greater than end index or equal!");
		}

		// Call a user function
		func(m_buffer + begin, end - begin);
	}

	// Output the contents of the buffer to std::cout stream
	void LinearBuffer::print() const
	{
		// Set up stream modifiers
		std::cout << std::hex << std::setfill('0') << std::uppercase;

		// Print bytes of the buffer
		for (unsigned int i = 0; i < m_size; ++i)
		{
			 std::cout << "0x" << std::setw(2) << (0xFF & m_buffer[i]) << ' ';
		}

		// Reset stream modifiers to the old ones
		std::cout << std::dec << std::setfill(' ') << std::nouppercase;

		// Output line feed
		std::cout << std::endl;
	}
}