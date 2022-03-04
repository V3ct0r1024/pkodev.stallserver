#include "Packet.h"
#include "RingBuffer.h"

#include <algorithm>

namespace pkodev
{
	// Constructor
	BasePacket::BasePacket()
	{
		// Reserve some memory for packet fields
		m_fields.reserve(8);
	}

	// Destructor
	BasePacket::~BasePacket()
	{

	}

	// Packet size
	std::size_t BasePacket::size() const
	{
		// Packet size counter
		std::size_t packet_size = 8; // 8 bytes - header size

		// Calculate the size of the packet as sum of size of all its fields
		std::for_each(
			m_fields.begin(),
			m_fields.end(),
			[&](const packet_field& field)
			{
				// Select the field type
				switch (field.type)
				{
					// 1 byte
					case packet_field_type_t::ubyte:
						{
							packet_size += sizeof(pkodev::ubyte_t);
						}
						break;

					// 1 byte unsigned int
					case packet_field_type_t::uint8:
						{
							packet_size += sizeof(pkodev::uint8_t);
						}
						break;

					// 2 bytes unsigned int
					case packet_field_type_t::uint16:
						{
							packet_size += sizeof(pkodev::uint16_t);
						}
						break;

					// 4 bytes unsigned int
					case packet_field_type_t::uint32:
						{
							packet_size += sizeof(pkodev::uint32_t);
						}
						break;

					// String (type of std::string)
					case packet_field_type_t::string:
						{
							packet_size += (*reinterpret_cast<pkodev::string_t*>(field.data)).length() + 1 + sizeof(pkodev::uint16_t);
						}
						break;

					// Byte array
					case packet_field_type_t::buffer:
						{
							packet_size += (*reinterpret_cast<byte_array_wrapper*>(field.data)).size + sizeof(pkodev::uint16_t);
						}
						break;
				}
			}
		);

		// Return packet size
		return packet_size;
	}

	// Write packet to a buffer
	std::size_t BasePacket::write(LinearBuffer& buffer) const
	{
		// Get packet size
		std::size_t packet_size = size();

		// Write packet header
		buffer.write_uint16(static_cast<pkodev::uint16_t>(packet_size));
		buffer.write_uint32(static_cast<pkodev::uint32_t>(0x80000000));
		buffer.write_uint16(static_cast<pkodev::uint16_t>(id()));

		// Write packet body
		std::for_each(
			m_fields.begin(),
			m_fields.end(),
			[&](const packet_field& field)
			{
				// Select the field type
				switch (field.type)
				{
					// 1 byte
					case packet_field_type_t::ubyte:
						{
							buffer.write_ubyte(*reinterpret_cast<pkodev::ubyte_t*>(field.data));
						}
						break;

					// 1 byte unsigned int
					case packet_field_type_t::uint8:
						{
							buffer.write_ubyte(*reinterpret_cast<pkodev::uint8_t*>(field.data));
						}
						break;

					// 2 bytes unsigned int
					case packet_field_type_t::uint16:
						{
							buffer.write_uint16(*reinterpret_cast<pkodev::uint16_t*>(field.data));
						}
						break;

					// 4 bytes unsigned int
					case packet_field_type_t::uint32:
						{
							buffer.write_uint32(*reinterpret_cast<pkodev::uint32_t*>(field.data));
						}
						break;

					// String (type of std::string)
					case packet_field_type_t::string:
						{
							buffer.write_string(*reinterpret_cast<pkodev::string_t*>(field.data));
						}
						break;

					// Byte array
					case packet_field_type_t::buffer:
						{
							const byte_array_wrapper& wrapper = *reinterpret_cast<byte_array_wrapper*>(field.data);
							buffer.write_uint16(static_cast<pkodev::uint16_t>(wrapper.size));
							buffer.write(wrapper.data, wrapper.size);
						}
						break;
				}
			}
		);

		// Return packet size
		return packet_size;
	}

	// Helper method for storing arrays of bytes
	void BasePacket::assign(const char* buf, std::size_t& size)
	{
		// Create a wrapper over the byte array and put it in the list of wrappers
		m_byte_arrays.push_back( { buf, size } );

		// Add the array of bytes to the list of packet fields
		m_fields.push_back( 
			{ 
				packet_field_type_t::buffer, 
				reinterpret_cast<void*>(&m_byte_arrays.back()) 
			}
		);
	}
}