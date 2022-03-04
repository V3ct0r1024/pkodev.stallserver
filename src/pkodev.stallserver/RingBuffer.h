#pragma once
#include "datatype.h"

#include <string>
#include <stdexcept>
#include <functional>

namespace pkodev
{
	// Ring buffer class exception
	class ring_buffer_exception final : public std::runtime_error
	{
		public:

			// Constructor with const char *
			ring_buffer_exception(const char* what) :
				std::runtime_error(what) { }

			// Constructor with std::string
			ring_buffer_exception(const std::string& what) :
				std::runtime_error(what) { }

	};
	
	// Ring buffer class
	class RingBuffer final
	{
		public:

			// Default buffer size 4K
			static const std::size_t default_size{ 4 * 1024 };


			// Constructor
			RingBuffer(std::size_t size = default_size);

			// Copy constructor
			RingBuffer(const RingBuffer&) = delete;

			// Move constructor
			RingBuffer(RingBuffer&&) = delete;

			// Destructor
			~RingBuffer();

			// Copy assignment operator
			RingBuffer& operator=(const RingBuffer&) = delete;

			// Move assignment operator
			RingBuffer& operator=(RingBuffer&&) = delete;

			// Get the number of bytes available for writing to the buffer
			std::size_t get_writeable_length() const;

			// Get the number of bytes available for reading from the buffer
			std::size_t get_readable_length() const;

			// Check if 'n' bytes can be written to the buffer
			bool can_write_bytes(std::size_t n) const;

			// Check if 'n' bytes can be read from the buffer
			bool can_read_bytes(std::size_t n) const;

			// Get the number of bytes written to the buffer
			std::size_t get_used_length() const;

			// Get buffer size
			inline std::size_t size() const { return m_size - 1; }

			// Write a block of data to the buffer
			void write(const char* data, std::size_t length);

			// Write operations
			void write_ubyte(ubyte_t ubyte);
			void write_uint8(uint8_t uint8);
			void write_uint16(uint16_t uint16);
			void write_uint32(uint32_t uint32);
			void write_string(const string_t& string);

			// Read a block of data from the buffer
			void read(char* dst, std::size_t length);

			// Read operations
			ubyte_t   read_ubyte();
			uint8_t   read_uint8();
			uint16_t  read_uint16();
			uint32_t  read_uint32();
			string_t  read_string();

			// Remove bytes that have been read from the buffer
			void read_commit();

			// Restart next read operation
			void read_rollback();

			// Skip reading of the specified number of bytes
			void read_skip(std::size_t n);

			// Clear buffer
			void clear();

		private:

			// Buffer size in bytes
			std::size_t m_size;

			// Write position
			std::size_t m_write_pos;

			// Read position
			std::size_t m_read_pos;

			// Data start position
			std::size_t m_data_pos;

			// Pointer to allocated memory for the buffer
			char* m_buffer;
	};
}