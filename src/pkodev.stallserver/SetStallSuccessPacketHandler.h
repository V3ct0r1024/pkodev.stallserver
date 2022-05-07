#pragma once
#include "PacketHandler.h"

namespace pkodev
{
	// Successful set stall installation packet handler
	// ID: 858
	// S -> C
	class SetStallSuccessPacketHandler final : public IPacketHandler
	{
		public:

			// Constructor
			SetStallSuccessPacketHandler();

			// Destructor
			~SetStallSuccessPacketHandler() override;

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

