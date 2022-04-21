#include "EnterMapPacketHandler.h"
#include "RingBuffer.h"
#include "Bridge.h"

namespace pkodev
{
	// Constructor
	EnterMapPacketHandler::EnterMapPacketHandler() :
		m_chaid(0),
		m_chaname(""),
		m_mapname("")
	{

	}

	// Destructor
	EnterMapPacketHandler::~EnterMapPacketHandler()
	{

	}

	// Packet ID
	unsigned short int EnterMapPacketHandler::id() const
	{
		return 516;
	}

	// Packet name
	std::string EnterMapPacketHandler::name() const
	{
		return std::string("Enter map");
	}

	// Transmission direction
	packet_direction_t EnterMapPacketHandler::direction() const
	{
		return packet_direction_t::sc;
	}

	// Read packet
	void EnterMapPacketHandler::read(std::size_t packet_size, LinearBuffer& buffer)
	{
		// Set read position on field with map name
		buffer.seek_read(6, LinearBuffer::seek_type::current);

		// Read map name
		m_mapname = buffer.read_string();

		// Set read position on field with character ID
		buffer.seek_read(5, LinearBuffer::seek_type::current);

		// Read character ID
		m_chaid = buffer.read_uint32();

		// Set read position on field with character name
		buffer.seek_read(4, LinearBuffer::seek_type::current);

		// Read character name
		m_chaname = buffer.read_string();
	}

	// Handle packet
	bool EnterMapPacketHandler::handle(Bridge& bridge)
	{
		// Reference on player's game data
		player_data& player = bridge.player();

		// Update the game data
		player.cha_id = m_chaid;
		player.cha_name = m_chaname;
		player.map = m_mapname;

		// Pass the packet further
		return true;
	}

	// Validate packet
	bool EnterMapPacketHandler::validate() const
	{
		return true;
	}
}