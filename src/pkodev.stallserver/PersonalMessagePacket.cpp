#include "PersonalMessagePacket.h"

namespace pkodev
{
	// Constructor
	PersonalMessagePacket::PersonalMessagePacket() :
		m_to_name(""),
		m_from_name(""),
		m_message("")
	{
		assign(m_to_name);
		assign(m_from_name);
		assign(m_message);
	}

	// Destructor
	PersonalMessagePacket::~PersonalMessagePacket()
	{

	}

	// Packet ID
	unsigned short int PersonalMessagePacket::id() const
	{
		return 5403;
	}

	// Set message data (sender, receiver, text)
	void PersonalMessagePacket::set_message(const std::string& from, 
		const std::string& to, const std::string& message)
	{
		m_from_name = from;
		m_to_name = to;
		m_message = message;
	}
}