#pragma once
#include "Packet.h"

namespace pkodev
{
	// GM notice packet (message to the GM chat channel)
	// ID: 5400
	// S -> C
	class GmNoticePacket final : public BasePacket
	{
	public:

		// Constructors
		GmNoticePacket();
		GmNoticePacket(const std::string& message);

		// Destructor
		~GmNoticePacket();

		// Packet ID
		unsigned short int id() const override;

		// Set message text
		inline void set_message(const std::string& msg) { m_message = msg; }

	private:

		// GM name
		std::string m_gm_name;

		// A message to the system chat channel
		std::string m_message;
	};
}

