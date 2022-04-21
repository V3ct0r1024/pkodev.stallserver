#include "PingRequestPacketHandler.h"
#include "RingBuffer.h"
#include "Bridge.h"


namespace pkodev
{
	// Constructor
	PingRequestPacketHandler::PingRequestPacketHandler()
	{

	}

	// Destructor
	PingRequestPacketHandler::~PingRequestPacketHandler()
	{

	}

	// Packet ID
	unsigned short int PingRequestPacketHandler::id() const
	{
		return 537;
	}

	// Packet name
	std::string PingRequestPacketHandler::name() const
	{
		return std::string("Ping request");
	}

	// Transmission direction
	packet_direction_t PingRequestPacketHandler::direction() const
	{
		return packet_direction_t::sc;
	}

	// Read packet
	void PingRequestPacketHandler::read(std::size_t packet_size, LinearBuffer& buffer)
	{

	}

	// Handle packet
	bool PingRequestPacketHandler::handle(Bridge& bridge)
	{
		// Check that Game.exe is disconnected and trading in offline stall
		if ( (bridge.game_connected() == false) && (bridge.player().set_stall == true) )
		{
			// Send ping response to the server
			bridge.send_packet_gate(m_response);

			// Do not pass the packet further
			return false;
		}

		// Pass the packet further
		return true;
	}

	// Validate packet
	bool PingRequestPacketHandler::validate() const
	{
		return true;
	}
}