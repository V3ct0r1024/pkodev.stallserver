#pragma once
#include "PacketHandler.h"
#include <string>

namespace pkodev
{
	// Map enter packet handler
	// ID: 516
	// S -> C
	class EnterMapPacketHandler final : public IPacketHandler
	{
		public:

			// Constructor
			EnterMapPacketHandler();

			// Destructor
			~EnterMapPacketHandler() override;

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

			// Character ID
			unsigned int m_chaid;

			// Character name
			std::string m_chaname;

			// Map name
			std::string m_mapname;
	};

}