#pragma once
#include "Packet.h"

namespace pkodev
{
	// Server disconnect packet
	// In response to this packet, GateServer.exe closes the connection
	// ID: 432
	// C -> S
	class DisconnectPacket final : public BasePacket
	{
		public:

			// Constructor
			DisconnectPacket();

			// Destructor
			~DisconnectPacket() override;

			// Packet ID
			unsigned short int id() const override;

	};
}