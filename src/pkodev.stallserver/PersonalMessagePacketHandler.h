#pragma once
#include "PacketHandler.h"
#include <string>

namespace pkodev
{
	// Personal message packet handler
	// ID: 6403
	// C -> S
	class PersonalMessagePacketHandler final : public IPacketHandler
	{
		public:

			// Constructor
			PersonalMessagePacketHandler();

			// Destructor
			~PersonalMessagePacketHandler();

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

			// Character name (recipient)
			std::string m_chaname;

			// Message
			std::string m_message;
	};
}
