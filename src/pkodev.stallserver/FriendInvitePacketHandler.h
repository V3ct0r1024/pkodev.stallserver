#pragma once
#include "PacketHandler.h"
#include <string>

namespace pkodev
{
	// Friend invitation packet handler
	// ID: 6011
	// C -> S
	class FriendInvitePacketHandler final : public IPacketHandler
	{
		public:

			// Constructor
			FriendInvitePacketHandler();

			// Destructor
			~FriendInvitePacketHandler();

			// Packet ID
			unsigned short int id() const override;

			// Packet name
			std::string name() const override;

			// Transmission direction
			packet_direction_t direction() const override;

			// Read packet
			void read(std::size_t packet_size, LinearBuffer& buffer) override;

			// Handle packet
			bool handle(Bridge& bridge) override;

			// Validate packet
			bool validate() const override;

		private:

			// Character name (invitee)
			std::string m_chaname;
	};
}
