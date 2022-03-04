#pragma once
#include "PacketHandler.h"

namespace pkodev
{
	// Set stall close packet handler
	// ID: 333
	// C -> S
	class SetStallClosePacketHandler final : public IPacketHandler
	{
		public:

			// Constructor
			SetStallClosePacketHandler();

			// Destructor
			~SetStallClosePacketHandler();

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

