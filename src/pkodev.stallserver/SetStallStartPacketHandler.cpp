#include "SetStallStartPacketHandler.h"
#include "Bridge.h"
#include "LinearBuffer.h"


namespace pkodev
{
	// Constructor
	SetStallStartPacketHandler::SetStallStartPacketHandler() :
		m_item_number(0)
	{

	}

	// Destructor
	SetStallStartPacketHandler::~SetStallStartPacketHandler()
	{

	}

	// Packet ID
	unsigned short int SetStallStartPacketHandler::id() const
	{
		return 330;
	}

	// Packet name
	std::string SetStallStartPacketHandler::name() const
	{
		return std::string("Set Stall open request");
	}
	// Transmission direction
	packet_direction_t SetStallStartPacketHandler::direction() const
	{
		return packet_direction_t::cs;
	}

	// Read packet
	void SetStallStartPacketHandler::read(std::size_t packet_size, LinearBuffer& buffer)
	{
		// Reset items number
		m_item_number = 0;
		
		// Skip stall name
		buffer.seek_read(static_cast<size_t>(buffer.read_uint16()), LinearBuffer::seek_type::current);

		// Read grids number
		const unsigned short int grids = static_cast<unsigned short int>(buffer.read_uint8());

		// Read grids
		for (unsigned int i = 0; i < static_cast<unsigned int>(min(grids, 18)); ++i)
		{
			// Skip 5 byte (grid and money)
			buffer.seek_read(5, LinearBuffer::seek_type::current);

			// Read item number
			m_item_number += static_cast<unsigned int>(buffer.read_uint8());

			// Skip 1 byte (index)
			buffer.seek_read(1, LinearBuffer::seek_type::current);
		}
	}

	// Handle packet
	bool SetStallStartPacketHandler::handle(Bridge& bridge)
	{
		// Set item number in set stall
		bridge.player().item_number = m_item_number;
		return true;
	}

	// Validate packet
	bool SetStallStartPacketHandler::validate() const
	{
		return true;
	}
}