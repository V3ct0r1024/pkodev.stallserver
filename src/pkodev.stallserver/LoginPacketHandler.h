#pragma once
#include "PacketHandler.h"

namespace pkodev
{
	// Server authorization packet handler
	// ID: 431
	// C -> S
	class LoginPacketHandler final : public IPacketHandler
	{
		public:

			// Constructor
			LoginPacketHandler();

			// Destructor
			~LoginPacketHandler();

			// Packet ID
			unsigned short int id() const override;

			// Packet name
			std::string name() const override;

			// Transmission direction
			packet_direction_t direction() const override;

			// Read packet
			void read(std::size_t packet_size, LinearBuffer& buffer) override;

			// Handle packet
			bool handle(Bridge& bridge) override;

			// Validate packet
			bool validate() const override;

		private:

			// Field "nobill"
			std::string m_nobill;

			// Login
			std::string m_login;

			// Password (MD5 hash)
			std::string m_password_md5;

			// MAC address
			std::string m_mac_address;

			// Field "flag"
			unsigned short int m_flag;

			// Game version
			unsigned short int m_version;
	};
}