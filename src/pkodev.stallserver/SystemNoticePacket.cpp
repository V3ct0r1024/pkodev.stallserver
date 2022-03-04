#include "SystemNoticePacket.h"

namespace pkodev
{
	// Constructor
	SystemNoticePacket::SystemNoticePacket() :
		m_message("")
	{
		assign(m_message);
	}

	// Destructor
	SystemNoticePacket::~SystemNoticePacket()
	{

	}

	// Packet ID
	unsigned short int SystemNoticePacket::id() const
	{
		return 517;
	}
}