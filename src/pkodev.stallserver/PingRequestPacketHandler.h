#pragma once
#include "PacketHandler.h"
#include "PingResponsePacket.h"

namespace pkodev
{
	// Ping request packet handler
	// ID: 537
	// S -> C
	class PingRequestPacketHandler final : public IPacketHandler
	{
		public:

			// Constructor
			PingRequestPacketHandler();

			// Destructor
			~PingRequestPacketHandler();

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

			// Ping response packet 
			PingResponsePacket m_response;

	};

}