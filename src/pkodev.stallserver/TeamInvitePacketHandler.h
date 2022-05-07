#pragma once
#include "PacketHandler.h"
#include <string>

namespace pkodev
{
	// Party invitation packet handler
	// ID: 6001
	// C -> S
	class TeamInvitePacketHandler final : public IPacketHandler
	{
		public:

			// Constructor
			TeamInvitePacketHandler();

			// Destructor
			~TeamInvitePacketHandler() override;

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
