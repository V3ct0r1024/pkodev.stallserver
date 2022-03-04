#include "LoginResultPacketHandler.h"
#include "RingBuffer.h"
#include "DES.h"
#include "CommEncrypt.h"

namespace pkodev
{
	// Constructor
	LoginResultPacketHandler::LoginResultPacketHandler() :
		m_ret(0),
		m_comm_encrypt(false),
		m_enc_key_length(0)
	{
		std::memset(reinterpret_cast<void*>(m_enc_key),    0x00, sizeof(m_enc_key));
		std::memset(reinterpret_cast<void*>(m_cs_dec_key), 0x00, common::enc_packet_key_len);
		std::memset(reinterpret_cast<void*>(m_cs_enc_key), 0x00, common::enc_packet_key_len);
		std::memset(reinterpret_cast<void*>(m_sc_dec_key), 0x00, common::enc_packet_key_len);
		std::memset(reinterpret_cast<void*>(m_sc_enc_key), 0x00, common::enc_packet_key_len);
	}

	// Destructor
	LoginResultPacketHandler::~LoginResultPacketHandler()
	{

	}

	// Packet ID
	unsigned short int LoginResultPacketHandler::id() const
	{
		return 931;
	}

	// Packet name
	std::string LoginResultPacketHandler::name() const
	{
		return std::string("Login result");
	}

	// Transmission direction
	packet_direction_t LoginResultPacketHandler::direction() const
	{
		return packet_direction_t::sc;
	}

	// Read packet
	void LoginResultPacketHandler::read(std::size_t packet_size, LinearBuffer& buffer)
	{
		// Read authorization error code
		m_ret = static_cast<unsigned short int>(buffer.read_uint16());

		// Check that player has successfully logged in
		if (m_ret == 0)
		{
			// Read session encryption key length
			m_enc_key_length = static_cast<std::size_t>(buffer.read_uint16());

			// Read session encryption key
			buffer.read(m_enc_key, m_enc_key_length);

			// Read packet encryption flag
			{
				// Set read position on field with encryption flag
				buffer.seek_read(packet_size - 8, LinearBuffer::seek_type::begin);

				// Read packet encryption flag
				m_comm_encrypt = static_cast<bool>(buffer.read_uint32());
			}
		}
	}

	// Handle packet
	bool LoginResultPacketHandler::handle(Bridge& bridge)
	{
		// Reference on player's game data
		player_data& data = bridge.player();

		// Update authorization flag
		data.authed = (m_ret == 0);

		// Check that player has successfully logged in
		if (data.authed == true)
		{
			// Set packet encryption flag
			data.comm_encrypt = m_comm_encrypt;

			// Check that packet encryption is enabled on the server
			if (data.comm_encrypt == true)
			{
				// Decrypt the session key
				common::CDES::RunDes(
					common::CDES::DECRYPT,
					common::CDES::ECB,
					m_enc_key,
					data.session_key, 
					m_enc_key_length,
					data.password_md5.c_str(),
					static_cast<unsigned char>(data.password_md5.length())
				);

				// Set length of the session key
				data.session_key_length = common::enc_session_key_len;

				// Make noise
				int noise = (data.version * data.version * 0x1232222) *
					(*reinterpret_cast<const int*>(data.chapstr.c_str() + data.chapstr.length() - 4));

				// Initialize packet encryption keys
				common::CommEncrypt::init_Noise(noise, m_cs_dec_key);  // 0 Расшифровывание - C -> S
				common::CommEncrypt::init_Noise(noise, m_sc_dec_key);  // 1 Расшифровывание - S -> C
				common::CommEncrypt::init_Noise(noise, m_cs_enc_key);  // 2 Шифрование      - C -> S
				common::CommEncrypt::init_Noise(noise, m_sc_enc_key);  // 3 Шифрование      - S -> С
				
				// Update packet encryption keys
				bridge.update_encrypt_keys(
					m_cs_enc_key, 
					m_cs_dec_key, 
					m_sc_enc_key, 
					m_sc_dec_key
				);
			}
		}

		// Pass the packet further
		return true;
	}

	// Validate packet
	bool LoginResultPacketHandler::validate() const
	{
		return true;
	}
}