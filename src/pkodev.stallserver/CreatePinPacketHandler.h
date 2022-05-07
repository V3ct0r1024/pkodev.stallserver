#pragma once
#include "PacketHandler.h"

namespace pkodev
{
	// Secret code (pin) creation packet handler
	// ID: 346
	// C -> S
	class CreatePinPacketHandler final : public IPacketHandler
	{
		public:

			// Constructor
			CreatePinPacketHandler();

			// Destructor
			~CreatePinPacketHandler() override;

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

			// Secret code (pin)
			std::string m_pin;

	};
}