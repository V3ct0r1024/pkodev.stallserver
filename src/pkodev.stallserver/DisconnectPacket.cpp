#include "DisconnectPacket.h"

namespace pkodev
{
	// Constructor
	DisconnectPacket::DisconnectPacket()
	{

	}

	// Destructor
	DisconnectPacket::~DisconnectPacket()
	{

	}

	// Packet ID
	unsigned short int DisconnectPacket::id() const
	{
		return 432;
	}
}