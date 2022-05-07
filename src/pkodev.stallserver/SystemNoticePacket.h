#pragma once
#include "Packet.h"

namespace pkodev
{
	// System notice packet (message to the system chat channel)
	// ID: 517
	// S -> C
	class SystemNoticePacket final : public BasePacket
	{
		public:

			// Constructors
			SystemNoticePacket();
			SystemNoticePacket(const std::string& message);

			// Destructor
			~SystemNoticePacket() override;

			// Packet ID
			unsigned short int id() const override;

			// Set message text
			inline void set_message(const std::string& msg) { m_message = msg; }

		private:

			// A message to the system chat channel
			std::string m_message;
	};
}