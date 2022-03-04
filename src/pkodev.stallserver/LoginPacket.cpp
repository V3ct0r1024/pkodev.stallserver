#include "LoginPacket.h"
#include "DES.h"

namespace pkodev
{
	// Constructor
	LoginPacket::LoginPacket() :
		m_nobill(""),
		m_login(""),
		m_mac_address(""),
		m_flag(0),
		m_version(0),
		m_chapstring(""),
		m_password_length(0)
	{
		// Initialize buffer for the encrypted password
		std::memset(reinterpret_cast<void*>(m_password_des), 0x00, sizeof(m_password_des));

		// Determine the packet structure
		assign(m_nobill);
		assign(m_login);
		assign(m_password_des, m_password_length);
		assign(m_mac_address);
		assign(m_flag);
		assign(m_version);
	}

	// Destructor
	LoginPacket::~LoginPacket()
	{

	}

	// Packet ID
	unsigned short int LoginPacket::id() const
	{
		return 431;
	}

	// Encrypt and set password
	void LoginPacket::set_password(const std::string& password)
	{
		// Padding buffer
		char pad[256]{ 0x00 };

		// Padding buffer length
		unsigned int pad_length = sizeof(pad);

		// Padding
		common::CDES::RunPad(
			common::CDES::PAD_ISO_1, 
			m_chapstring.c_str(), 
			m_chapstring.length(), 
			pad, 
			pad_length
		);

		// Encryption
		common::CDES::RunDes(
			common::CDES::ENCRYPT, 
			common::CDES::ECB,
			pad, 
			m_password_des,
			pad_length, 
			password.c_str(),
			static_cast<unsigned char>(password.length())
		);

		// Get encrypted password length
		m_password_length = pad_length;
	}
}