#include "PingResponsePacket.h"

namespace pkodev
{
	// Constructor
	PingResponsePacket::PingResponsePacket()
	{

	}

	// Destructor
	PingResponsePacket::~PingResponsePacket()
	{

	}

	// Packet ID
	unsigned short int PingResponsePacket::id() const
	{
		return 17;
	}
}