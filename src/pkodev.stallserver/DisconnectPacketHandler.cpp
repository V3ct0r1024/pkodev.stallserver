#include "DisconnectPacketHandler.h"
#include "RingBuffer.h"
#include "Bridge.h"

namespace pkodev
{
	// Contructor
	DisconnectPacketHandler::DisconnectPacketHandler()
	{

	}

	// Destructor
	DisconnectPacketHandler::~DisconnectPacketHandler()
	{

	}

	// Packet ID
	unsigned short int DisconnectPacketHandler::id() const
	{
		return 432;
	}

	// Packet name
	std::string DisconnectPacketHandler::name() const
	{
		return std::string("Disconnect request");
	}

	// Transmission direction
	packet_direction_t DisconnectPacketHandler::direction() const
	{
		return packet_direction_t::cs;
	}

	// Read packet
	void DisconnectPacketHandler::read(std::size_t packet_size, LinearBuffer& buffer)
	{

	}

	// Handle packet
	bool DisconnectPacketHandler::handle(Bridge& bridge)
	{
		// Check that the player is in offline stall
		if (bridge.player().set_stall == true)
		{
			// Do not pass the packet further
			return false;
		}

		// Pass the packet further
		return true;
	}

	// Validate packet
	bool DisconnectPacketHandler::validate() const
	{
		return true;
	}
}