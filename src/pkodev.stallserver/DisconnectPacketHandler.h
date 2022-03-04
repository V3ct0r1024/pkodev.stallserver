#pragma once
#include "PacketHandler.h"

namespace pkodev
{
	// Disconnect packet handler
	// ID: 432
	// C -> S
	class DisconnectPacketHandler final : public IPacketHandler
	{
		public:

			// Contructor
			DisconnectPacketHandler();

			// Destructor
			~DisconnectPacketHandler();

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

	};
}


