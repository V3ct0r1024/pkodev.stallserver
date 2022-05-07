#pragma once
#include "PacketHandler.h"

namespace pkodev
{
	// Set stall open packet handler
	// ID: 330
	// C -> S
	class SetStallStartPacketHandler final : public IPacketHandler
	{
		public:

			// Constructor
			SetStallStartPacketHandler();

			// Destructor
			~SetStallStartPacketHandler() override;

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

			// Item number
			unsigned int m_item_number;

	};
}
