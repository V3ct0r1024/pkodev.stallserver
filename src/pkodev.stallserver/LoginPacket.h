#pragma once
#include "Packet.h"
#include "Utils.h"

namespace pkodev
{
	// Server authorization packet
	// ID: 431
	// C -> S
	class LoginPacket final : public BasePacket
	{
		public:

			// Constructor
			LoginPacket();

			// Destructor
			~LoginPacket();

			// Packet ID
			unsigned short int id() const override;

			// Set packet fields (account details)
			inline void set_nobill(const std::string& nobill) { m_nobill = nobill; }
			inline void set_login(const std::string& login) { m_login = login; }
			void set_password(const std::string& password);
			inline void set_mac_address(const std::string& mac_address) { m_mac_address = mac_address; }
			inline void set_flag(unsigned short int flag) { m_flag = flag; }
			inline void set_ip_address(const std::string& ip) { m_ip_address = utils::network::ip_address_to_int(ip); }
			inline void set_version(unsigned short int version) { m_version = version; }

			// Set string with connection time
			inline void set_chapstring(const std::string& chapstring) { m_chapstring = chapstring; }

		private:

			// Field "nobill"
			std::string m_nobill;

			// Login
			std::string m_login;

			// MAC-address
			std::string m_mac_address;

			// IP address (GateServer.exe IP mod)
			unsigned int m_ip_address;

			// Field "flag"
			unsigned short int m_flag;

			// Game version
			unsigned short int m_version;

			// String with connection time
			std::string m_chapstring;

			// Buffer for encrypted password
			char m_password_des[128];

			// Encrypted password length
			std::size_t m_password_length;
	};
}
