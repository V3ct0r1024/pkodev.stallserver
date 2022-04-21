#include "SetStallDelPacketHandler.h"
#include "Bridge.h"
#include "LinearBuffer.h"
#include <iostream>

namespace pkodev
{
	// Constructor
	SetStallDelPacketHandler::SetStallDelPacketHandler() :
		m_chaid(0),
		m_item_number(0)
	{

	}

	// Destructor
	SetStallDelPacketHandler::~SetStallDelPacketHandler()
	{

	}

	// Packet ID
	unsigned short int SetStallDelPacketHandler::id() const
	{
		return 856;
	}

	// Packet name
	std::string SetStallDelPacketHandler::name() const
	{
		return std::string("Set Stall item bought");
	}

	// Transmission direction
	packet_direction_t SetStallDelPacketHandler::direction() const
	{
		return packet_direction_t::sc;
	}

	// Read packet
	void SetStallDelPacketHandler::read(std::size_t packet_size, LinearBuffer& buffer)
	{
		// Read character ID
		m_chaid = buffer.read_uint32();

		// Skip grid ID
		buffer.seek_read(1, LinearBuffer::seek_type::current);

		// Read item number
		m_item_number = static_cast<unsigned int>(buffer.read_uint8());
	}

	// Handle packet
	bool SetStallDelPacketHandler::handle(Bridge& bridge)
	{	
		// Get a reference to the player related data
		player_data& player = bridge.player();

		// Check character ID and that it is trading in set stall
		if ( (player.cha_id == m_chaid) && (player.set_stall == true) )
		{
			// Update items number
			player.item_number -= m_item_number;

			// Check that stall is empty and 'close_stall_on_empty' parameter is enabled
			if ( (player.item_number == 0) && (bridge.server().settings().close_stall_on_empty == true) )
			{
				// Check that client is not connected
				if (bridge.game_connected() == false)
				{
					// Close offline stall
					bridge.disconnect();
				}
				else
				{
					// Reset 'set stall' flag
					player.set_stall = false;
				}
			}
		}

		return true;
	}

	// Validate packet
	bool SetStallDelPacketHandler::validate() const
	{
		return true;
	}
}