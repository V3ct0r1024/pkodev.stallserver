#pragma once
#include "Bridge.h"
#include "PacketHandler.h"

namespace pkodev
{
	// Authorization result packet handler
	// ID: 931
	// S -> C
	class LoginResultPacketHandler final : public IPacketHandler
	{
		public:

			// Constructor
			LoginResultPacketHandler();

			// Destructor
			~LoginResultPacketHandler();

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

			// Authorization error code
			unsigned short int m_ret;

			// Is packet encryption enabled on the server
			bool m_comm_encrypt;

			// Session encryption key length
			std::size_t m_enc_key_length;

			// Session encryption key buffer
			char m_enc_key[16];

			// Packet encryption keys
			char m_cs_dec_key[common::enc_packet_key_len];
			char m_cs_enc_key[common::enc_packet_key_len];
			char m_sc_dec_key[common::enc_packet_key_len];
			char m_sc_enc_key[common::enc_packet_key_len];
	};

}
