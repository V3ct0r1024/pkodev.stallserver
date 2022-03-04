#include "FriendInvitePacketHandler.h"
#include "LinearBuffer.h"
#include "Bridge.h"

namespace pkodev
{
	// Constructor
	FriendInvitePacketHandler::FriendInvitePacketHandler() :
		m_chaname("")
	{

	}

	// Destructor
	FriendInvitePacketHandler::~FriendInvitePacketHandler()
	{

	}

	// Packet ID
	unsigned short int FriendInvitePacketHandler::id() const
	{
		return 6011;
	}

	// Packet name
	std::string FriendInvitePacketHandler::name() const
	{
		return std::string("Friend invite packet");
	}

	// Transmission direction
	packet_direction_t FriendInvitePacketHandler::direction() const
	{
		return packet_direction_t::cs;
	}

	// Read packet
	void FriendInvitePacketHandler::read(std::size_t packet_size, LinearBuffer& buffer)
	{
		m_chaname = buffer.read_string();
	}

	// Handle packet
	bool FriendInvitePacketHandler::handle(Bridge& bridge)
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
	bool FriendInvitePacketHandler::validate() const
	{
		return true;
	}
}