#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <stdexcept>

#include <event2/event.h>

#include "Server.h"
#include "ObjectPool.h"
#include "Timer.h"
#include "RingBuffer.h"
#include "LinearBuffer.h"
#include "PacketHandlerStorage.h"
#include "Packet.h"
#include "LoginPacket.h"

#include <iostream>

namespace pkodev
{
	// Define some constants
	namespace common
	{
		// Maximum packet size
		const std::size_t max_packet_size = 4096;

		// Session encryption key length
		const std::size_t enc_session_key_len = 6;

		// Packet encryption key length
		const std::size_t enc_packet_key_len = 4;

		// Session ID
		const unsigned int SESSION_ID_80000000 = 0x80000000;
		const unsigned int SESSION_ID_00000001 = 0x00000001;

		// The length of the packet identifier (ID) interval
		const unsigned short int CMD_INTERVAL = 500;

		// Packet ID intervals
		const unsigned short int CMD_CM_BASE = 0;      // Client      -> GameServer
		const unsigned short int CMD_MC_BASE = 500;    // GameServer  -> Client
		const unsigned short int CMD_PC_BASE = 5000;   // GroupServer -> Client
		const unsigned short int CMD_CP_BASE = 6000;   // Client	  -> GroupServer

		// ID of packet with connection time string from GateServer.exe
		const unsigned short int CMD_MC_CHAPSTR = 940;

		// ID of login packet from Game.exe
		const unsigned short int CMD_CM_LOGIN = 431;
	};

	// Determine type of pointer to the list of packet handlers
	typedef std::shared_ptr<PacketHandlerStorage> handlers_ptr;

	// Determine type of pointer to the timer
	typedef std::unique_ptr<Timer> timer_ptr;

	// Network bridge side type
	enum class endpoint_type_t : unsigned short int
	{
		game = 0,                // Game.exe
		gate                     // GateServer.exe
	};

	// Network bridge class exception
	class bridge_exception final : public std::runtime_error
	{
		public:

			// Constructor with const char *
			bridge_exception(const char *what) :
				std::runtime_error(what) { }

			// Constructor with std::string
			bridge_exception(const std::string& what) :
				std::runtime_error(what) { }

	};

	// Network address structure
	struct address final
	{
		// IP address
		std::string ip;

		// Port
		unsigned short int port;

		// Constructor
		address() :
			ip(""), port(0) { }

		// Constructor
		address(const std::string& ip_, unsigned short int port_) :
			ip(ip_), port(port_) { }
	};

	// A player's data structure
	struct player_data final
	{
		// Is the player logged in
		bool authed;

		// Game version (from GateServer.cfg)
		unsigned short int version;

		// Is the player in offline stall mode
		bool set_stall;

		// Number of items in offline stall
		unsigned int item_number;

		// Is packet encryption enabled
		bool comm_encrypt;
	
		// Session encryption key length
		std::size_t session_key_length;

		// Session encryption key
		char session_key[16];

		// String with the time of connection to the server
		std::string chapstr;

		// Login
		std::string login;

		// Password (MD5 hash)
		std::string password_md5;

		// Current map (garner, magicsea, darkblue, ...)
		std::string map;

		// Character ID
		unsigned int cha_id;

		// Character nickname
		std::string cha_name;

		// Reconnect flag
		bool reconnecting;

		// Login packet
		LoginPacket login_packet;

		// Constructor
		player_data() :
			authed(false),
			version(0),
			set_stall(false),
			item_number(0),
			comm_encrypt(false),
			chapstr(""),
			login(""),
			password_md5(""),
			map(""),
			cha_id(0),
			cha_name(""),
			reconnecting(false),
			session_key_length(0)
		{
			std::memset(reinterpret_cast<void*>(session_key), 0x00, sizeof(session_key));
		}
	};

	// Network bridge class between Game.exe and GateServer.exe
	class Bridge final : public IPoolable
	{
		public:

			// Constructor
			Bridge(Server& server);

			// Copy constructor
			Bridge(const Bridge&) = delete;

			// Move constructor
			Bridge(Bridge&&) = delete;

			// Destructor 
			~Bridge() override;

			// Copy assignment operator
			Bridge& operator=(const Bridge&) = delete;

			// Move assignment operator
			Bridge& operator=(Bridge&&) = delete;

			// Build a network bridge
			void build(const Server::worker& w, evutil_socket_t fd, 
				const address& game, const address& gate);

			// Remove a network bridge
			void reset() override;

			// Connect to GateServer.exe
			void connect();

			// Disconnect from GateServer.exe
			void disconnect();

			// Get Game.exe network address
			inline const address& game_address() const { return m_client_ctx.addr; }

			// Get GateServer.exe network address
			inline const address& gate_address() const { return m_server_ctx.addr; }

			// Check that Game.exe is connected
			inline bool game_connected() const { return m_client_ctx.connected; }

			// Check that GateServer.exe is connected
			inline bool gate_connected() const { return m_server_ctx.connected; }

			// Send a packet to Game.exe
			inline bool send_packet_game(const IPacket& packet) { return send_packet(m_client_ctx, packet); }

			// Send a packet to GateServer.exe
			inline bool send_packet_gate(const IPacket& packet) { return send_packet(m_server_ctx, packet); }

			// Get a reference to the server instance
			inline Server& server() const { return m_server; }

			// Get a reference to the player's data
			inline player_data& player() { return m_player_data; }
			inline const player_data& player() const { return m_player_data; }

			// Update packet encryption keys
			void update_encrypt_keys(const char* cs_enc, const char* cs_dec,
				const char* sc_enc, const char* sc_dec);

			// Get mutex
			std::recursive_mutex& get_lock() const { return m_mtx; }

		private:

			// Data for exchange with the network bridge side
			struct endpoint final
			{
				// Socket descriptor
				evutil_socket_t socket;

				// Connection state
				bool connected;

				// Number of received packets
				std::size_t packet_counter;

				// Read event
				event* read_event;

				// Write event
				event* write_event;

				// Address
				address addr;

				// Input network buffer for incoming data
				std::unique_ptr<RingBuffer> recv_buf;

				// Buffer for incoming packets processing 
				std::unique_ptr<LinearBuffer> in_buf;

				// Output network buffer for outgoing data
				std::unique_ptr<RingBuffer> send_buf;

				// Buffer for outgoing packets processing 
				std::unique_ptr<LinearBuffer> out_buf;

				// Packet encryption keys
				char enc_recv_key[common::enc_packet_key_len];  // Receive
				char enc_send_key[common::enc_packet_key_len];  // Send

				// Constructor
				endpoint();

				// Destructor 
				~endpoint();
			};

			// Socket read event
			bool on_read(endpoint& to, endpoint& from, endpoint_type_t side);

			// Socket write event
			bool on_write(endpoint& to, endpoint& from, endpoint_type_t side);

			// Connection close event
			void on_disconnect(endpoint_type_t side);

			// GateServer.exe connected event
			bool on_connect();

			// Send a packet to the network bridge side
			bool send_packet(endpoint& ctx, const IPacket& packet);


			// Reference to the server instance
			Server& m_server;

			// Pointer to event base structure
			event_base* m_event_base;

			// Player authorization timer
			timer_ptr m_auth_timer;

			// Player trade timer
			timer_ptr m_trade_timer;

			// Data for exchange with Game.exe
			endpoint m_client_ctx;

			// Data for exchange with GateServer.exe
			endpoint m_server_ctx;

			// Received packet handlers
			handlers_ptr m_handlers;

			// Game logic related data
			player_data m_player_data;

			// Mutex
			mutable std::recursive_mutex m_mtx;

			// Disconnection flag
			bool m_disconnecting;

			// Reading flag
			bool m_reading;

			// Writing flag
			bool m_writing;
	};

	// Network bridge class creator
	class BridgeMaker final : public IPoolableMaker
	{
		public:

			// Constructor
			BridgeMaker(Server& server) :
				m_server(server) { }

			// Destructor
			~BridgeMaker() override = default;

			// Create a new network bridge
			IPoolable* create() override
			{
				return new Bridge(m_server);
			}

		private:

			// Reference to the server instance
			Server& m_server;
	};
}


