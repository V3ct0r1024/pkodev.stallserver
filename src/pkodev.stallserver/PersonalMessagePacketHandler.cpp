#include "PersonalMessagePacketHandler.h"
#include "LinearBuffer.h"
#include "Bridge.h"
#include "PersonalMessagePacket.h"

namespace pkodev
{
	// Constructor
	PersonalMessagePacketHandler::PersonalMessagePacketHandler() :
		m_chaname(""),
		m_message("")
	{

	}

	// Destructor
	PersonalMessagePacketHandler::~PersonalMessagePacketHandler()
	{

	}

	// Packet ID
	unsigned short int PersonalMessagePacketHandler::PersonalMessagePacketHandler::id() const
	{
		return 6403;
	}

	// Packet name
	std::string PersonalMessagePacketHandler::PersonalMessagePacketHandler::name() const
	{
		return std::string("Personal message");
	}

	// Transmission direction
	packet_direction_t PersonalMessagePacketHandler::PersonalMessagePacketHandler::direction() const
	{
		return packet_direction_t::cs;
	}

	// Read packet
	void PersonalMessagePacketHandler::read(std::size_t packet_size, LinearBuffer& buffer)
	{
		m_chaname = buffer.read_string();
		m_message = buffer.read_string();
	}

	// Handle packet
	bool PersonalMessagePacketHandler::handle(Bridge& bridge)
	{
		// Look for recipient in the list of offline bridges
		auto recipient = bridge.server().offline_bridges().find_by_character(m_chaname);

		// Check that the recipient was found
		if (recipient)
		{
			// Create packet with a personal message for sender
			PersonalMessagePacket packet;

			// Set message
			packet.set_message(bridge.player().cha_name, m_chaname, "This player is offline now!");

			// Send packet to the sender
			bridge.send_packet_game(packet);

			// Recipient is offline
			return false;
		}

		// Recipient is online
		return true;
	}

	// Validate packet
	bool PersonalMessagePacketHandler::validate() const
	{
		return true;
	}
}