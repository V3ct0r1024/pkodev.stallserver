#pragma once
#include <string>
#include "PacketHandler.h"

namespace pkodev
{
	// Chap string packet handler
	// ID: 940
	// S -> C
	class ChapStringPacketHandler final : public IPacketHandler
	{
		public:

			// Constructor
			ChapStringPacketHandler();

			// Destructor
			~ChapStringPacketHandler();

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

			// String with date and time of connection to the server
			std::string m_chapstring;
	};
}


