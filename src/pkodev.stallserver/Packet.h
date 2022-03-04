#pragma once
#include "LinearBuffer.h"

#include <string>
#include <vector>

namespace pkodev
{
	// Game logic layer network packet interface 
	class IPacket
	{
		public:

			// Packet ID
			virtual unsigned short int id() const = 0;

			// Packet size
			virtual std::size_t size() const = 0;

			// Write packet to a buffer
			virtual std::size_t write(LinearBuffer&) const = 0;
	};

	// Basic network packet
	class BasePacket : public IPacket
	{
		public:

			// Constructor
			BasePacket();

			// Destructor
			virtual ~BasePacket();

			// Packet size
			std::size_t size() const override;

			// Write packet to a buffer
			std::size_t write(LinearBuffer& buffer) const override;


		protected:

			// Packet field type
			enum class packet_field_type_t
			{
				ubyte = 0,   // 1 byte
				uint8,       // 1 byte unsigned int
				uint16,      // 2 bytes unsigned int
				uint32,      // 4 bytes unsigned int
				string,      // String (type of std::string)
				buffer,		 // Byte array
			};

			// Packet field structure
			struct packet_field final
			{
				// Field type
				packet_field_type_t type;

				// Data pointer
				void* data;

				// Constructor
				packet_field(packet_field_type_t type_, void* data_) :
					type(type_), 
					data(data_)
				{

				}
			};

			// Wrapper structure over a byte array
			struct byte_array_wrapper final
			{
				// Array length
				std::size_t& size;

				// Data pointer
				const char* data;

				// Constructor
				byte_array_wrapper(const char* data_, std::size_t& size_) :
					size(size_),
					data(data_)
				{

				}
			};

			// Helper operations for registering packet fields
			void assign(pkodev::ubyte_t&  byte)   { m_fields.push_back({ packet_field_type_t::ubyte,  reinterpret_cast<void* >(&byte)   }); }
			void assign(pkodev::uint8_t&  uint8)  { m_fields.push_back({ packet_field_type_t::uint8,  reinterpret_cast<void* >(&uint8)  }); }
			void assign(pkodev::uint16_t& uint16) { m_fields.push_back({ packet_field_type_t::uint16, reinterpret_cast<void* >(&uint16) }); }
			void assign(pkodev::uint32_t& uint32) { m_fields.push_back({ packet_field_type_t::uint32, reinterpret_cast<void* >(&uint32) }); }
			void assign(pkodev::string_t& str)    { m_fields.push_back({ packet_field_type_t::string, reinterpret_cast<void* >(&str   ) }); }
			void assign(const char* buf, std::size_t& size);

			// Packet fields list
			std::vector<packet_field> m_fields;

			// Helper list for storing arrays of bytes
			std::vector<byte_array_wrapper> m_byte_arrays;
	};
}
