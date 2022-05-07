#pragma once
#include "PacketHandler.h"

namespace pkodev
{
	// Secret code (pin) changing packet handler
	// ID: 347
	// C -> S
	class UpdatePinPacketHandler final : public IPacketHandler
	{
		public:

			// Constructor
			UpdatePinPacketHandler();

			// Destructor
			~UpdatePinPacketHandler() override;

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

			// Old PIN
			std::string m_old_pin;

			// New PIN
			std::string m_new_pin;

	};
}