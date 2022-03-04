#pragma once
#include "PacketHandler.h"
#include <string>

namespace pkodev
{
	// Chat session creation packet handler
	// ID: 6406
	// C -> S
	class TalkSessionCreatePacketHandler final : public IPacketHandler
	{
		public:

			// Constructor
			TalkSessionCreatePacketHandler();

			// Destructor
			~TalkSessionCreatePacketHandler();

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

			// The number of players in a chat session
			std::size_t m_chanum;

			// Character name (invitee)
			std::string m_chaname;
	};
}
