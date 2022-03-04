#pragma once
#include "Packet.h"

namespace pkodev
{
	// Personal message packet
	// ID: 5403
	// S -> C
	class PersonalMessagePacket final : public BasePacket
	{
		public:

			// Constructor
			PersonalMessagePacket();

			// Destructor
			~PersonalMessagePacket();

			// Packet ID
			unsigned short int id() const override;

			// Set message data (sender, receiver, text)
			void set_message(const std::string& from, const std::string& to, const std::string& message);

		private:

			// Sender
			std::string m_from_name;

			// Receiver
			std::string m_to_name;

			// Text
			std::string m_message;
	};
}