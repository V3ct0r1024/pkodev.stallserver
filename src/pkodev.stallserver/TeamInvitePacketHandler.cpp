#include "TeamInvitePacketHandler.h"
#include "LinearBuffer.h"
#include "Bridge.h"

namespace pkodev
{
	// Constructor
	TeamInvitePacketHandler::TeamInvitePacketHandler() :
		m_chaname("")
	{

	}

	// Destructor
	TeamInvitePacketHandler::~TeamInvitePacketHandler()
	{

	}

	// Packet ID
	unsigned short int TeamInvitePacketHandler::id() const
	{
		return 6001;
	}

	// Packet name
	std::string TeamInvitePacketHandler::name() const
	{
		return std::string("Team invite packet");
	}

	// Transmission direction
	packet_direction_t TeamInvitePacketHandler::direction() const
	{
		return packet_direction_t::cs;
	}

	// Read packet
	void TeamInvitePacketHandler::read(std::size_t packet_size, LinearBuffer& buffer)
	{
		m_chaname = buffer.read_string();
	}

	// Handle packet
	bool TeamInvitePacketHandler::handle(Bridge& bridge)
	{
		// Look for an invitee in the list of offline bridges
		auto invited = bridge.server().offline_bridges().find_by_character(m_chaname);

		// Check that the invitee was found
		if (invited)
		{
			// Send a message to the inviter
			bridge.system_notice("This player is offline now!");

			// The invitee is offline
			return false;
		}

		// The invitee is online
		return true;
	}

	// Validate packet
	bool TeamInvitePacketHandler::validate() const
	{
		return true;
	}
}