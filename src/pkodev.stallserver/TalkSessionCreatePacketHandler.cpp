#include "TalkSessionCreatePacketHandler.h"
#include "SystemNoticePacket.h"
#include "LinearBuffer.h"
#include "Bridge.h"

namespace pkodev
{
	// Constructor
	TalkSessionCreatePacketHandler::TalkSessionCreatePacketHandler() :
		m_chanum(0),
		m_chaname("")
	{

	}

	// Destructor
	TalkSessionCreatePacketHandler::~TalkSessionCreatePacketHandler()
	{

	}

	// Packet ID
	unsigned short int TalkSessionCreatePacketHandler::id() const
	{
		return 6406;
	}

	// Packet name
	std::string TalkSessionCreatePacketHandler::name() const
	{
		return std::string("Talk session create packet");
	}

	// Transmission direction
	packet_direction_t TalkSessionCreatePacketHandler::direction() const
	{
		return packet_direction_t::cs;
	}

	// Read packet
	void TalkSessionCreatePacketHandler::read(std::size_t packet_size, LinearBuffer& buffer)
	{
		// Read number of players in a chat session
		m_chanum = static_cast<std::size_t>(buffer.read_ubyte());

		// Check that there is one player in the chat session
		if (m_chanum == 1)
		{
			// Read character name (invitee)
			m_chaname = buffer.read_string();
		}
	}

	// Handle packet
	bool TalkSessionCreatePacketHandler::handle(Bridge& bridge)
	{
		// Check that there is one player in the chat session
		if (m_chanum == 1)
		{
			// Look for an invitee in the list of offline bridges
			auto opt = bridge.server().offline_bridges().find_by_character(m_chaname);

			// Check that the invitee was found
			if (opt.has_value() == true)
			{
				// Send a message to the inviter
				bridge.send_packet_game(SystemNoticePacket("This player is offline now!"));

				// The invitee is offline
				return false;
			}
		}

		// The invitee is online
		return true;
	}

	// Validate packet
	bool TalkSessionCreatePacketHandler::validate() const
	{
		return true;
	}
}