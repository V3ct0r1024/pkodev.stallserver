#include "GmNoticePacket.h"

namespace pkodev
{
	// Constructor
	GmNoticePacket::GmNoticePacket() :
		m_gm_name("Stall Server"),
		m_message("")
	{
		assign(m_gm_name);
		assign(m_message);
	}

	// Constructor
	GmNoticePacket::GmNoticePacket(const std::string& message) :
		m_gm_name("Stall Server"),
		m_message(message)
	{
		assign(m_gm_name);
		assign(m_message);
	}

	// Destructor
	GmNoticePacket::~GmNoticePacket()
	{

	}

	// Packet ID
	unsigned short int GmNoticePacket::id() const
	{
		return 5400;
	}
}