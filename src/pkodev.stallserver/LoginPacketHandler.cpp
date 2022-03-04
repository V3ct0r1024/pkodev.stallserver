#include "LoginPacketHandler.h"
#include "RingBuffer.h"
#include "Bridge.h"
#include "Utils.h"
#include "DES.h"
#include "DisconnectPacket.h"


namespace pkodev
{
	// Constructor
	LoginPacketHandler::LoginPacketHandler() :
		m_nobill(""),
		m_login(""),
		m_password_md5(""),
		m_mac_address(""),
		m_flag(0),
		m_version(0)
	{

	}

	// Destructor
	LoginPacketHandler::~LoginPacketHandler()
	{

	}

	// Packet ID
	unsigned short int LoginPacketHandler::id() const
	{
		return 431;
	}

	// Packet name
	std::string LoginPacketHandler::name() const
	{
		return std::string("Login packet");
	}

	// Transmission direction
	packet_direction_t LoginPacketHandler::direction() const
	{
		return packet_direction_t::cs;
	}

	// Read packet
	void LoginPacketHandler::read(std::size_t packet_size, LinearBuffer& buffer)
	{
		m_nobill       = static_cast<std::string>(buffer.read_string());
		m_login        = static_cast<std::string>(buffer.read_string());
		m_password_md5 = static_cast<std::string>(buffer.read_string());
		m_mac_address  = static_cast<std::string>(buffer.read_string());
		m_flag         = static_cast<unsigned short int>(buffer.read_uint16());
		m_version      = static_cast<unsigned short int>(buffer.read_uint16());
	}

	// Handle packet
	bool LoginPacketHandler::handle(Bridge& bridge)
	{
		// Reconnect flag
		bool relogin = false;
		
		// Looking for account in the list of offline stalls
		auto other = bridge.server().offline_bridges().find_by_account(m_login);

		// Check that the account is in an offline stall now
		if (other)
		{
			// Get a reference to game data of player in offline stall
			player_data& other_data = (*other)->player();

			{
				// Check password
				if (other_data.password_md5 == m_password_md5)
				{
					// Create disconnect packet
					DisconnectPacket disconnect;

					// Raise reconnection flag
					other_data.reconnecting = true;

					// Wait for reconnection
					relogin = true;

					// Send packet to disconnect player in offline stall
					bool ret = (*other)->send_packet_gate(disconnect);
				}
			}
		}
		
		// Get a reference to game data of connecting player
		player_data& data = bridge.player();

		// Update new player data
		data.login = m_login;
		data.password_md5 = m_password_md5;
		data.version = m_version;

		// Build login packet
		data.login_packet.set_chapstring(data.chapstr);
		data.login_packet.set_nobill(m_nobill);
		data.login_packet.set_login(m_login);
		data.login_packet.set_password(m_password_md5);
		data.login_packet.set_mac_address(m_mac_address);
		data.login_packet.set_flag(m_flag);
		data.login_packet.set_version(m_version);

		// Check that reconnection is not required
		if (relogin == false)
		{
			// Send login packet to the server
			bridge.send_packet_gate(data.login_packet);
		}

		// Do not pass the packet further
		return false;
	}

	// Validate packet
	bool LoginPacketHandler::validate() const
	{
		// Check MAC address
		return utils::validation::check_mac_address(m_mac_address);
	}
}