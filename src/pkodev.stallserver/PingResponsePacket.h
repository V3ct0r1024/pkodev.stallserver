#pragma once
#include "Packet.h"

namespace pkodev
{
	// Ping request from GameServer.exe response packet
	// ID: 17
	// C -> S
	class PingResponsePacket final : public BasePacket
	{
		public:

			// Constructor
			PingResponsePacket();

			// Destructor
			~PingResponsePacket() override;

			// Packet ID
			unsigned short int id() const override;

	};
}


