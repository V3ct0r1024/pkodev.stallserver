#include "SetStallClosePacketHandler.h"
#include "RingBuffer.h"
#include "Bridge.h"

namespace pkodev
{
	// Constructor
	SetStallClosePacketHandler::SetStallClosePacketHandler()
	{

	}

	// Destructor
	SetStallClosePacketHandler::~SetStallClosePacketHandler()
	{

	}

	// Packet ID
	unsigned short int SetStallClosePacketHandler::id() const
	{
		return 333;
	}

	// Packet name
	std::string SetStallClosePacketHandler::name() const
	{
		return std::string("Set Stall close request");
	}

	// Transmission direction
	packet_direction_t SetStallClosePacketHandler::direction() const
	{
		return packet_direction_t::cs;
	}

	// Read packet
	void SetStallClosePacketHandler::read(std::size_t packet_size, LinearBuffer& buffer)
	{

	}

	// Handle packet
	bool SetStallClosePacketHandler::handle(Bridge& bridge)
	{
		// Reset the flag of set stall state
		bridge.player().set_stall = false;

		// Pass the packet further
		return true;
	}

	// Validate packet
	bool SetStallClosePacketHandler::validate() const
	{
		return true;
	}
}