#pragma once
#include "datatype.h"

#include <string>
#include <stdexcept>
#include <functional>

namespace pkodev
{

	// Linear buffer class exception
	class linear_buffer_exception final : public std::runtime_error
	{
		public:

			// Constructor with const char *
			linear_buffer_exception(const char *what) :
				std::runtime_error(what) { }

			// Constructor with std::string
			linear_buffer_exception(const std::string& what) :
				std::runtime_error(what) { }

	};

	// Linear buffer class
	class LinearBuffer final
	{
		public:

			// Read / write position seek mode
			enum class seek_type
			{
				begin = 0,    // Relative to the beginning
				current,      // Relative to the current position
				end           // Relative to the ending
			};

			// Type for custom data processing function
			typedef std::function<void(char*, std::size_t)> user_cb_t;

			// Default buffer size 4K
			static const std::size_t default_size{ 4 * 1024 };


			// Constructor
			LinearBuffer(std::size_t size = default_size);

			// Copy constructor
			LinearBuffer(const LinearBuffer&) = delete;

			// Move constructor
			LinearBuffer(LinearBuffer&&) = delete;

			// Destructor
			~LinearBuffer();

			// Copy assignment operator
			LinearBuffer& operator=(const LinearBuffer&) = delete;

			// Move assignment operator
			LinearBuffer& operator=(LinearBuffer&&) = delete;

			// Write a block of data to the buffer
			void write(const char* data, std::size_t length);

			// Write operations
			void write_ubyte(ubyte_t ubyte);
			void write_uint8(uint8_t uint8);
			void write_uint16(uint16_t uint16);
			void write_uint32(uint32_t uint32);
			void write_string(const string_t& string);
			void write_bytearray(const ubyte_t* data, uint16_t size);

			// Read a block of data from the buffer
			void read(char* dst, std::size_t length);

			// Read operations
			ubyte_t  read_ubyte();
			uint8_t  read_uint8();
			uint16_t read_uint16();
			uint32_t read_uint32();
			string_t read_string();
			uint16_t read_bytearray(ubyte_t* dst, std::size_t size);

			// Get a pointer to data in the buffer
			inline const char* data() const { return m_buffer; }

			// Get byte by index
			ubyte_t byte(std::size_t index) const;

			// Overloaded operator [] (get byte by index)
			ubyte_t operator[](std::size_t index) const { return byte(index); }

			// Get buffer size
			inline std::size_t size() const { return m_size; }

			// Get read position
			inline std::size_t tell_read() const { return m_read_pos; }

			// Get write position
			inline std::size_t tell_write() const { return m_write_pos; }

			// Set read position
			void seek_read(std::size_t index, seek_type type = seek_type::begin);

			// Set write position
			void seek_write(std::size_t index, seek_type type = seek_type::begin);

			// Check that there are 'length' bytes in the buffer to write
			bool can_write_bytes(std::size_t length) const;

			// Check that there are 'length' bytes in the buffer to read
			bool can_read_bytes(std::size_t length) const;

			// Clear buffer
			void clear();

			// Apply a custom function to data in the buffer
			void apply(std::size_t begin, std::size_t end, user_cb_t func);

			// Output the contents of the buffer to std::cout stream
			void print() const;

		private:

			// Buffer size in bytes
			std::size_t m_size;

			// Write position
			std::size_t m_write_pos;

			// Read position
			std::size_t m_read_pos;

			// Pointer to allocated memory for the buffer
			char* m_buffer;
	};
}

