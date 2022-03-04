#include "ChapStringPacketHandler.h"
#include "RingBuffer.h"
#include "Bridge.h"


namespace pkodev
{
	// Constructor
	ChapStringPacketHandler::ChapStringPacketHandler() :
		m_chapstring("")
	{

	}

	// Destructor
	ChapStringPacketHandler::~ChapStringPacketHandler()
	{

	}

	// Packet ID
	unsigned short int ChapStringPacketHandler::id() const
	{
		return 940;
	}

	// Packet name
	std::string ChapStringPacketHandler::name() const
	{
		return std::string("Chap string");
	}

	// Transmission direction
	packet_direction_t ChapStringPacketHandler::direction() const
	{
		return packet_direction_t::sc;
	}

	// Read packet
	void ChapStringPacketHandler::read(std::size_t packet_size, LinearBuffer& buffer)
	{
		m_chapstring = static_cast<std::string>(buffer.read_string());
	}

	// Handle packet
	bool ChapStringPacketHandler::handle(Bridge& bridge)
	{
		// Update player's game data
		bridge.player().chapstr = m_chapstring;

		// Pass the packet further
		return true;
	}

	// Validate packet
	bool ChapStringPacketHandler::validate() const
	{
		return true;
	}
}