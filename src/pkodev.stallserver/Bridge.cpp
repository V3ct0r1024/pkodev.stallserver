#include "Bridge.h"
#include "Packet.h"
#include "CommEncrypt.h"
#include "Utils.h"
#include "Logger.h"

namespace pkodev
{
	// Constructor of a data structure for network exchange with the network bridge side
	Bridge::endpoint::endpoint() :
		socket(INVALID_SOCKET),
		read_event(nullptr), write_event(nullptr),
		connected(false),
		recv_buf(std::make_unique<RingBuffer>(4 * common::max_packet_size)),
		in_buf(std::make_unique<LinearBuffer>(common::max_packet_size)),
		send_buf(std::make_unique<RingBuffer>(4 * common::max_packet_size)),
		out_buf(std::make_unique<LinearBuffer>(common::max_packet_size)),
		packet_counter(0)
	{
		std::memset(reinterpret_cast<void*>(enc_recv_key), 0x00, common::enc_packet_key_len);
		std::memset(reinterpret_cast<void*>(enc_send_key), 0x00, common::enc_packet_key_len);
	}

	// Destructor of a data structure for network exchange with the network bridge side
	Bridge::endpoint::~endpoint()
	{
		// Close the socket
		if (connected == true) 
		{ 
			evutil_closesocket(socket);
		}

		// Delete socket read event
		if (read_event != nullptr) 
		{ 
			event_free(read_event); 
			read_event = nullptr;
		}

		// Delete socket write event
		if (write_event != nullptr) 
		{
			event_free(write_event); 
			write_event = nullptr;
		}
	}



	// Network bridge constructor
	Bridge::Bridge(Server& server) :
		m_server(server),
		m_event_base(nullptr)
	{
		// Set types of bridge sides
		m_server_ctx.type = endpoint_type_t::gate;
		m_client_ctx.type = endpoint_type_t::game;
	}

	// Network bridge destructor
	Bridge::~Bridge()
	{

	}

	// Build a network bridge
	void Bridge::build(const Server::worker& w, evutil_socket_t fd, 
		const ip_address& game, const ip_address& gate)
	{
		// Save a pointer to the event loop
		m_event_base = w.evbase;

		// Save a pointer to the packet handlers
		m_handlers = w.handlers;

		// Initialize the data structure for network communication with Game.exe
		m_client_ctx.socket = fd;
		m_client_ctx.address = game;
		m_client_ctx.connected = true;

		// Initialize the data structure for network communication with GateServer.exe
		m_server_ctx.socket = INVALID_SOCKET;
		m_server_ctx.address = gate;
		m_server_ctx.connected = false;

		// Create an authorization timer
		{
			// Create the timer
			m_auth_timer = std::make_unique<Timer>(w.evbase, "Authentication timer");
			
			// Define a function that will be called when the timer expires 
			m_auth_timer->on_timer(
				[&]()
				{
					// Lock the bridge
					std::lock_guard<std::recursive_mutex> lock(m_mtx);

					// Check that a player is authorized
					if (m_player_data.authed == false)
					{
						// Kick the player from the server
						on_disconnect(endpoint_type_t::gate);
					}
				}
			);
		}
		
		// Create an offline stall timer
		if (m_server.settings().max_offline_time > 0)
		{
			// Create the timer
			m_trade_timer = std::make_unique<Timer>(w.evbase, "Offline stall timer");

			// Define a function that will be called when the timer expires 
			m_trade_timer->on_timer(
				[&]() 
				{ 
					// Lock the bridge
					std::lock_guard<std::recursive_mutex> lock(m_mtx);

					// Kick the player from the server
					on_disconnect(endpoint_type_t::gate);
				}
			);
		}
	}

	// Remove a network bridge
	void Bridge::reset()
	{
		// Remove timers
		{
			// Authorization timer
			m_auth_timer.reset();

			// Offline stall timer
			m_trade_timer.reset();
		}
		
		// Reset data structures for exchange with Game.exe and GateServer.exe
		{
			// Reset lambda
			auto ctx_reset = [](endpoint& ctx)
			{
				// Close socket
				if (ctx.connected == true)
				{
					// Close
					evutil_closesocket(ctx.socket);

					// Not connected
					ctx.connected = false;
				}

				// Clear buffers
				ctx.recv_buf->clear();  // Input network buffer for incoming data
				ctx.send_buf->clear();  // Output network buffer for outgoing data
				ctx.in_buf->clear(); 	// Buffer for incoming packets processing 
				ctx.out_buf->clear();   // Buffer for outgoing packets processing 

				// Reset the address
				ctx.address.ip = "0.0.0.0";
				ctx.address.port = 0;

				// Reset received packets counter
				ctx.packet_counter = 0;

				// Remove packet encryption keys
				std::memset(reinterpret_cast<void*>(ctx.enc_recv_key), 0x00, common::enc_packet_key_len);
				std::memset(reinterpret_cast<void*>(ctx.enc_send_key), 0x00, common::enc_packet_key_len);
			};

			// GateServer.exe
			ctx_reset(m_server_ctx);

			// Game.exe
			ctx_reset(m_client_ctx);
		}

		// Reset the pointer to the event loop
		m_event_base = nullptr;

		// Reset the pointer to the packet handlers loop
		m_handlers.reset();

		// Reset game logic data
		{
			m_player_data.authed = false;
			m_player_data.version = 0;
			m_player_data.offline_stall = false;
			m_player_data.comm_encrypt = false;
			m_player_data.chapstr = "";
			m_player_data.login = "";
			m_player_data.password_md5 = "";
			m_player_data.map = "";
			m_player_data.cha_name = "";
			m_player_data.reconnecting = false;
			m_player_data.session_key_length = 0;
			std::memset(reinterpret_cast<void*>(m_player_data.session_key), 0x00, sizeof(m_player_data.session_key));
		}
	}

	// Connect to GateServer.exe
	void Bridge::connect()
	{
		// Check that Game.exe is connected
		if (m_client_ctx.connected == false)
		{
			throw bridge_exception("Can't connect to GateServer because Game is disconnected.");
		}

		// Check that GateServer.exe is disconnected from the network bridge
		if (m_server_ctx.connected == true)
		{
			throw bridge_exception("Can't connect to GateServer because GateServer already connected.");
		}

		// Create a socket for GateServer.exe
		m_server_ctx.socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		// Check that the GateServer.exe socket was created
		if (m_server_ctx.socket == INVALID_SOCKET)
		{
			// socket() function error
			throw bridge_exception(
				std::string("Failed to create GateServer socket (socket() error). "
					"Error code: " + std::to_string(WSAGetLastError()))
			);
		}

		// Making the GateServer.exe socket non-blocking
		int ret = evutil_make_socket_nonblocking(m_server_ctx.socket);

		// Check that the GateServer.exe socket is switched to non-blocking mode
		if (ret == SOCKET_ERROR)
		{
			// evutil_make_socket_nonblocking() function error
			throw bridge_exception(
				std::string("Failed to make GateServer socket non-blocking (evutil_make_socket_nonblocking() error). "
					"Error code: " + std::to_string(WSAGetLastError()))
			);
		}
		
		// Fill in GateServer.exe address structure
		sockaddr_in remote;
		std::memset(reinterpret_cast<void*>(&remote), 0x00, sizeof(remote));
		remote.sin_family = AF_INET;
		remote.sin_port = htons(m_server_ctx.address.port);
		ret = inet_pton(AF_INET, m_server_ctx.address.ip.c_str(), &remote.sin_addr);

		// Check the result of the inet_pton() function
		if (ret == SOCKET_ERROR)
		{
			// inet_pton() function error
			throw bridge_exception(
				std::string("Failed to convert GateServer IP address into its numeric binary form (inet_pton() error). "
					"Error code: " + std::to_string(WSAGetLastError()))
			);
		}

		// Callback function for events on the Game.exe socket
		auto client_cb = [](evutil_socket_t fd, short int what, void* ctx) noexcept
		{
			// Catch all exceptions
			try
			{
				// Reference to the network bridge
				Bridge& bridge = *reinterpret_cast<Bridge*>(ctx);

				// Lock the bridge
				std::lock_guard<std::recursive_mutex> lock(bridge.m_mtx);

				// References to client and server data structures
				endpoint& client = bridge.m_client_ctx;
				endpoint& server = bridge.m_server_ctx;

				// Check socket read event
				if (what & EV_READ)
				{
					// Handle the read event
					const bool ret = bridge.on_read(server, client);

					// Check if the event was successfully handled
					if (ret == false)
					{
						// Close the connection with Game.exe
						bridge.on_disconnect(endpoint_type_t::game);
					}
				}

				// Check socket write event
				if (what & EV_WRITE)
				{
					// Handle the write event
					const bool ret = bridge.on_write(client, server);

					// Check if the event was successfully handled
					if (ret == false)
					{
						// Close the connection with Game.exe
						bridge.on_disconnect(endpoint_type_t::game);
					}
				}
			}
			catch (...)
			{
				// Write a message to the log
				Logger::Instance().log("Caught an exception in client callback method!");
			}
		};

		// Callback function for events on the GateServer.exe socket
		auto server_cb = [](evutil_socket_t fd, short int what, void* ctx) noexcept
		{
			// Catch all exceptions
			try
			{
				// Reference to the network bridge
				Bridge& bridge = *reinterpret_cast<Bridge*>(ctx);

				// Lock the bridge
				std::lock_guard<std::recursive_mutex> lock(bridge.m_mtx);

				// References to client and server data structures
				endpoint& client = bridge.m_client_ctx;
				endpoint& server = bridge.m_server_ctx;

				// Check socket read event
				if (what & EV_READ)
				{
					// Handle the read event
					const bool ret = bridge.on_read(client, server);

					// Check if the event was successfully handled
					if (ret == false)
					{
						// Close the connection with GateServer.exe
						bridge.on_disconnect(endpoint_type_t::gate);
					}
				}

				// Check socket write event
				if (what & EV_WRITE)
				{
					// Check that the connection to GateServer.exe is in progress
					if (server.connected == false)
					{
						// GateServer.exe socket error code
						const int error = utils::network::get_socket_error(server.socket);

						// Check that GateServer.exe connected without errors
						if (error == 0)
						{
							// Connected to GateServer.exe
							const bool ret = bridge.on_connect();

							// Check that connect event handling succeed
							if (ret == false)
							{
								// Close the connection with GateServer.exe
								bridge.on_disconnect(endpoint_type_t::gate);
							}
						}
						else
						{
							// Failed to connect to GateServer.exe due to network error
							Logger::Instance().log("Can't connect to GateServer because network error occurred! Error code: %d", error);
						
							// Close the connection with Game.exe
							bridge.on_disconnect(endpoint_type_t::game);
						}
					}
					else
					{
						// Handle the write event
						const bool ret = bridge.on_write(server, client);

						// Check if the event was successfully handled
						if (ret == false)
						{
							// Close the connection with GateServer.exe
							bridge.on_disconnect(endpoint_type_t::gate);
						}
					}
				}
			}
			catch (...)
			{
				// Write a message to the log
				Logger::Instance().log("Caught an exception in server callback method!");
			}
		};

		// Create read and write events for Game.exe socket
		{
			// Read event
			m_client_ctx.write_event = event_new(m_event_base, m_client_ctx.socket, EV_WRITE, client_cb, reinterpret_cast<void*>(this));
			
			// Write event
			m_client_ctx.read_event = event_new(m_event_base, m_client_ctx.socket, EV_READ, client_cb, reinterpret_cast<void*>(this));

			// Check that events for Game.exe socket have been created
			if ( (m_client_ctx.write_event == nullptr) || (m_client_ctx.read_event == nullptr) )
			{
				throw bridge_exception(
					"Failed to create Game write and read events (event_new() error)."
				);
			}
		}

		// Create read and write events for GateServer.exe socket
		{
			// Read event
			m_server_ctx.write_event = event_new(m_event_base, m_server_ctx.socket, EV_WRITE, server_cb, reinterpret_cast<void*>(this));
			
			// Write event	
			m_server_ctx.read_event = event_new(m_event_base, m_server_ctx.socket, EV_READ, server_cb, reinterpret_cast<void*>(this));

			// Check that events for GateServer.exe socket have been created
			if ( (m_server_ctx.write_event == nullptr) || (m_server_ctx.read_event == nullptr) )
			{
				throw bridge_exception(
					"Failed to create GateServer write and read events (event_new() error)."
				);
			}
		}

		// Connect to GateServer.exe
		ret = ::connect(m_server_ctx.socket, reinterpret_cast<const sockaddr*>(&remote), sizeof(remote));
		
		// Check the result of the connect() function
		if (ret == SOCKET_ERROR)
		{
			// Check if the socket wants to block
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				// connect() function error
				throw bridge_exception(
					std::string("Failed to connect to the server (connect() error). "
						"Error code: " + std::to_string(WSAGetLastError()))
				);
			}
		}

		// Add a write event for GateServer.exe socket to the event loop
		ret = event_add(m_server_ctx.write_event, nullptr);

		// Check the result of the connect() function
		if (ret == -1)
		{
			// event_add() function error
			throw bridge_exception(
				"Failed to add write event to the event loop (event_add() error)."
			);
		}
	}

	// Update packet encryption keys
	void Bridge::update_encrypt_keys(const char* cs_enc, const char* cs_dec,
		const char* sc_enc, const char* sc_dec)
	{
		std::memcpy(m_client_ctx.enc_recv_key, cs_dec, common::enc_packet_key_len);
		std::memcpy(m_client_ctx.enc_send_key, cs_enc, common::enc_packet_key_len);
		std::memcpy(m_server_ctx.enc_recv_key, sc_enc, common::enc_packet_key_len);
		std::memcpy(m_server_ctx.enc_send_key, sc_dec, common::enc_packet_key_len);
	}

	// GateServer.exe connected event
	bool Bridge::on_connect()
	{
		// Add read events to the event loop
		{
			// GateServer.exe
			int ret = event_add(m_server_ctx.read_event, nullptr);

			// Check event_add() result
			if (ret == -1)
			{
				// Write a message to log
				Logger::Instance().log("Failed to add GateServer socket read event after establishing connection with the server!");
				return false;
			}

			// Game.exe
			ret = event_add(m_client_ctx.read_event, nullptr);

			// Check event_add() result
			if (ret == -1)
			{
				// Write a message to log
				Logger::Instance().log("Failed to add Game socket read event after establishing connection with the server!");
				return false;
			}
		}

		// Start the authorization timer for 2.048 seconds
		bool ret = m_auth_timer->start(2048);

		// Check the result
		if (ret == false)
		{
			// Write a message to log
			Logger::Instance().log("Failed to start authorization timer after establishing connection with the server!");
			return false;
		}
		
		// Connected to GateServer.exe
		m_server_ctx.connected = true;

		// Success
		return true;
	}

	// Socket read event
	bool Bridge::on_read(endpoint& to, endpoint& from)
	{
		// Temporary buffer for reading data from socket
		char tmp[common::max_packet_size];

		// Read data from socket into the temporary buffer
		const int ret = ::recv(from.socket, tmp, sizeof(tmp), 0);

		// Check that connection was closed by other side
		if (ret == 0)
		{
			// Close connection
			return false;
		}

		// Check that an error occurred during the network exchange
		if (ret == SOCKET_ERROR)
		{
			// Check if the socket wants to block
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				// Close connection
				return false;
			}

			// Wait for data read event again
			const int result = event_add(from.read_event, nullptr);

			// Check that event is added to the event loop
			if (result == -1)
			{
				// Write a message to log
				Logger::Instance().log("Failed to add socket read event in read callback on WSAEWOULDBLOCK error!");

				// Close connection
				return false;
			}

			// Read event handled successfully
			return true;
		}

		// Write the received data to the input ring buffer
		try
		{
			// Write the received data block to the input buffer
			from.recv_buf->write(tmp, ret);

			// Split the data in the input buffer into packets
			while (from.recv_buf->can_read_bytes(2) == true)
			{

				// Read packet size
				const std::size_t packet_size = static_cast<std::size_t>(from.recv_buf->read_uint16());

				// Check packet size
				if ((packet_size < 2) || (packet_size != 2 && packet_size < 6))
				{
					// Incorrect packet size
					return false;
				}

				// Check that packet size does not exceed the buffer size
				if (packet_size > from.recv_buf->size())
				{
					// Packet size exceeds buffer size
					return false;
				}

				// Check that entire packet has accumulated in the buffer except for 2 bytes of length
				if (from.recv_buf->can_read_bytes(packet_size - 2) == false)
				{
					// Restart next read operation
					from.recv_buf->read_rollback();

					// Wait for data read event again
					const int result = event_add(from.read_event, nullptr);

					// Check that event is added to the event loop
					if (result == -1)
					{
						// Write a message to log
						Logger::Instance().log("Failed to add socket read event in read callback while waiting for more data!");

						// Close connection
						return false;
					}

					// Wait for more data
					return true;
				}

				// The need to forward the packet further
				bool pass = true;

				// Is packet encryption enabled
				const bool comm_encrypt = m_player_data.comm_encrypt;

				// Process the packet
				if (packet_size > 2)
				{
					// Restart next read operation
					from.recv_buf->read_rollback();

					// Clear the processing buffer
					from.in_buf->clear();

					// The number of bytes to copy
					std::size_t counter = packet_size;

					// Copy the packet from the input buffer to the processing buffer
					while (counter != 0)
					{
						// Determine the minimum size of the data block
						std::size_t block_size = min(counter, sizeof(tmp));

						// Read a block of data from the input ring buffer
						from.recv_buf->read(tmp, block_size);

						// Write a block of data to the input line buffer
						from.in_buf->write(tmp, block_size);

						// Reduce the number of packet bytes that are left to copy
						counter -= block_size;
					}

					// Check if packet encryption is enabled
					if (comm_encrypt == true)
					{
						// Decrypt the packet
						from.in_buf->apply(
							6,
							packet_size,
							[&](char* data, std::size_t size)
							{
								common::CommEncrypt::encrypt_B(data, size, m_player_data.session_key, m_player_data.session_key_length, false);
								common::CommEncrypt::decrypt_Noise(to.enc_recv_key, data, size);
							}
						);
					}

					// Skip 2 bytes (packet length)
					from.in_buf->seek_read(2);

					// Get session ID
					const unsigned int session_id = static_cast<unsigned int>(from.in_buf->read_uint32());

					// Check session ID
					if ((session_id != common::SESSION_ID_80000000) && (session_id != common::SESSION_ID_00000001))
					{
						// Unknown session ID
						return false;
					}

					// Get packet ID
					const unsigned short int packet_id = static_cast<unsigned short int>(from.in_buf->read_uint16());

					// Check packet ID
					if (
						(packet_id == 0) ||
						(
							((packet_id > common::CMD_CM_BASE && packet_id < (common::CMD_CM_BASE + common::CMD_INTERVAL)) == false) &&
							((packet_id > common::CMD_MC_BASE && packet_id < (common::CMD_MC_BASE + common::CMD_INTERVAL)) == false) &&
							((packet_id > common::CMD_PC_BASE && packet_id < (common::CMD_PC_BASE + common::CMD_INTERVAL)) == false) &&
							((packet_id > common::CMD_CP_BASE && packet_id < (common::CMD_CP_BASE + common::CMD_INTERVAL)) == false)
						)
					)
					{
						// Unknown packet ID
						return false;
					}

					// Check that ID of the first packet is 940 (GateServer.exe) or 431 (Game.exe) 
					if ( (from.packet_counter == 0) &&
						 ( (packet_id != common::CMD_MC_CHAPSTR) && (packet_id != common::CMD_CM_LOGIN) )
					)
					{
						// Incorrect the first packet
						return false;
					}

					// Check that the packet exists in the list of handlers
					if (m_handlers->check_handler(packet_id) == true)
					{
						// Get a pointer to the handler
						auto handler = m_handlers->get_handler(packet_id);

						// Read the packet
						handler->read(packet_size, *(from.in_buf));

						// Validate the packet
						if (handler->validate() == true)
						{
							// Proccess the packet
							pass = handler->handle(*(this));
						}
						else
						{
							// The packet is not valid
							return false;
						}
					}

					// Check if packet encryption is enabled
					if ( (pass == true) && (comm_encrypt == true) )
					{
						// Encrypt the packet
						from.in_buf->apply(
							6,
							packet_size,
							[&](char* data, std::size_t size)
							{
								common::CommEncrypt::encrypt_Noise(to.enc_send_key, data, size);
								common::CommEncrypt::encrypt_B(data, size, m_player_data.session_key, m_player_data.session_key_length, true);
							}
						);
					}
				}

				// Check the other side is connected
				if (to.connected == true)
				{
					// Check the packet length
					if (packet_size > 2)
					{
						// Check that the packet needs to be transferred further
						if (pass == true)
						{
							// Restart all read operations in the input packet processing buffer
							from.in_buf->seek_read(0);

							// The number of bytes to copy
							std::size_t counter = packet_size;

							// Copy the packet from the input packet processing buffer to the output buffer of other side
							while (counter != 0)
							{
								// Determine the minimum size of the data block
								std::size_t block_size = min(counter, sizeof(tmp));

								// Read a block of data from the input line buffer
								from.in_buf->read(tmp, block_size);

								// Write a block of data to the output ring buffer
								to.send_buf->write(tmp, block_size);

								// Reduce the number of packet bytes that are left to copy
								counter -= block_size;
							}
						}
					}
					else
					{
						// Check that sender request a ping response
						if (packet_size == 2)
						{
							// Write a ping response to the output buffer of sender side
							to.send_buf->write_uint16(2);
						}
					}
				}
				else
				{
					// Check that sender request a ping response
					if (packet_size == 2)
					{
						// Write a ping response to the output buffer of sender side
						from.send_buf->write_uint16(2);
					}
				}
				
				// Remove the packet from the input buffer
				from.recv_buf->read_commit();

				// Increase the counter of received packets
				++from.packet_counter;

			} // Packets splitting loop

			// Send the data to the other side
			if (to.connected == true)
			{
				// The other side is connected, transfer the data further
				if (to.send_buf->get_readable_length() > 0)
				{
					// Add read event
					const int result = event_add(to.write_event, nullptr);

					// Check that event is added to the event loop
					if (result == -1)
					{
						// Write a message to log
						Logger::Instance().log("Failed to add socket read event in read callback while transferring the data to the receiver side!");

						// Close connection
						return false;
					}
				}
			}
			else
			{
				// The other side is disconnected, transfer the data to the sender side
				if (from.send_buf->get_readable_length() > 0)
				{
					// Add read event
					const int result = event_add(from.write_event, nullptr);

					// Check that event is added to the event loop
					if (result == -1)
					{
						// Write a message to log
						Logger::Instance().log("Failed to add socket read event in read callback while transferring the data to the sender side!");

						// Close connection
						return false;
					}
				}
			}

			// Wait for data read event again
			const int result = event_add(from.read_event, nullptr);

			// Check that event is added to the event loop
			if (result == -1)
			{
				// Write a message to log
				Logger::Instance().log("Failed to add socket read event in read callback after handling!");

				// Close connection
				return false;
			}

			// Read event handled successfully
			return true;
		}
		catch (const ring_buffer_exception& e)
		{
			Logger::Instance().log("Exception 'ring_buffer_exception' is occurred on socket read callback: %s", e.what());
		}
		catch (const linear_buffer_exception& e)
		{
			Logger::Instance().log("Exception 'linear_buffer_exception' is occurred on socket read callback: %s", e.what());
		}
		catch (const packet_handler_storage_exception& e)
		{
			Logger::Instance().log("Exception 'packet_handler_storage_exception' is occurred on socket read callback: %s", e.what());
		}
		catch (const std::exception& e)
		{
			Logger::Instance().log("Exception 'std::exception' is occurred on socket read callback: %s", e.what());
		}
		catch (...)
		{
			Logger::Instance().log("Exception '...' is occurred on socket read callback");
		}

		// Errors occurred while handling the read event
		return false;
	}

	// Socket write event
	bool Bridge::on_write(endpoint& to, endpoint& from)
	{
		// Write the data from output buffer to socket
		try
		{
			// The number of bytes in the output buffer
			const std::size_t n = to.send_buf->get_readable_length();

			// Check that the output buffer has data to send
			if (n == 0)
			{
				// Output buffer is empty
				return true;
			}

			// Temporary buffer for writing data to socket
			char tmp[common::max_packet_size];

			// Calculate the minimum size of the data block to read from the output buffer
			const std::size_t block_size = min(sizeof(tmp), n);

			// Read data into the temporary buffer
			to.send_buf->read(tmp, block_size);

			// Write data to socket
			const int ret = ::send(to.socket, tmp, block_size, 0);

			// Check that connection was closed by other side
			if (ret == 0)
			{
				// Close connection
				return false;
			}

			// Check that an error occurred during the network exchange
			if (ret == SOCKET_ERROR)
			{
				// Check if the socket wants to block
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					// Close connection
					return false;
				}

				// Wait for a data write event again
				const int result = event_add(to.write_event, nullptr);

				// Check that event is added to the event loop
				if (result == -1)
				{
					// Write a message to log
					Logger::Instance().log("Failed to add socket write event in write callback on WSAEWOULDBLOCK error!");

					// Close connection
					return false;
				}

				// Read event handled successfully
				return true;
			}

			// Remove the sent data block from the output buffer
			to.send_buf->read_commit();

			// Check that there is data in the output buffer
			if (to.send_buf->get_readable_length() > 0)
			{
				// Wait for a data write event again
				const int result = event_add(to.write_event, nullptr);

				// Check that event is added to the event loop
				if (result == -1)
				{
					// Write a message to log
					Logger::Instance().log("Failed to add socket write event in write callback after handling!");

					// Close connection
					return false;
				}
			}

			// Read event handled successfully
			return true;
		}
		catch (const ring_buffer_exception& e)
		{
			Logger::Instance().log("Exception 'ring_buffer_exception' is occurred on socket write callback: %s", e.what());
		}
		catch (const linear_buffer_exception& e)
		{
			Logger::Instance().log("Exception 'linear_buffer_exception' is occurred on socket write callback: %s", e.what());
		}
		catch (const std::exception& e)
		{
			Logger::Instance().log("Exception 'std::exception' is occurred on socket write callback: %s", e.what());
		}
		catch (...)
		{
			Logger::Instance().log("Exception '...' is occurred on socket write callback");
		}

		// Errors occurred while handling the write event
		return false;
	}

	// Connection close event
	void Bridge::on_disconnect(endpoint_type_t side)
	{
		// Close connection lambda
		auto close = [](endpoint& ctx)
		{
			// Remove events from the event loop
			{
				// Remove read event 
				event_del(ctx.read_event);
				event_free(ctx.read_event);

				// Remove read event 
				event_del(ctx.write_event);
				event_free(ctx.write_event);
			}

			// Close the socket
			evutil_closesocket(ctx.socket);

			// Reset connection flag
			ctx.connected = false;
		};

		// Check remote side type (GateServer.exe or Game.exe)
		switch (side)
		{
			// Game.exe
			case endpoint_type_t::game:
				{
					// Close Game.exe socket
					close(m_client_ctx);
				
					// Check that player is not in offline stall
					if (m_player_data.offline_stall == true)
					{
						// Add the bridge to the list of offline stalls
						bool ret = m_server.offline_bridges().add(this);

						// Check result
						if (ret == true)
						{
							// Offline trade time limit from settings file
							const unsigned int trade_time = m_server.settings().max_offline_time;

							// Check that the limit is enabled
							if (trade_time > 0)
							{
								// Start offline trade timer
								ret = m_trade_timer->start(static_cast<unsigned long long>(trade_time) * 1000);

								// Check that the timer has been started
								if (ret == false)
								{
									// Write a message to the log
									Logger::Instance().log("Failed to launch a timer to limit trade time on the offline stall!");

									// Close the connection with GateServer.exe
									close(m_server_ctx);
								}
							}
						}
						else
						{
							// Write a message to the log
							Logger::Instance().log("Failed to add a bridge to the offline stall list!");

							// Close the connection with GateServer.exe
							close(m_server_ctx);
						}
					}
					else
					{
						// Close the connection with GateServer.exe
						close(m_server_ctx);
					}
				}
				break;

			// GateServer.exe
			case endpoint_type_t::gate:
				{
					// Close GateServer.exe socket
					close(m_server_ctx);

					if (m_client_ctx.connected == true)
					{
						// Close Game.exe socket
						close(m_client_ctx);
					}

					// Remove the bridge from the offline stalls list
					if (m_player_data.offline_stall == true)
					{
						// Update the offline stalls list
						const bool ret = m_server.offline_bridges().remove(this);

						// Check result
						if (ret == false)
						{
							// Write a message to the log
							Logger::Instance().log("Failed to remove a bridge from the offline stall list!");
						}
					}

					// Check reconnect flag
					if (m_player_data.reconnecting == true)
					{
						// Search a bridge in the common list of network bridges by login
						auto opt = m_server.bridges().find(
							[&](const Bridge& other_bridge) -> bool
							{
								// Get a reference to game data of connecting player
								const player_data& other_data = other_bridge.player();

								// Check that the player is not authorized
								if (other_data.authed == false)
								{
									// Compare accounts
									return (
										utils::string::lower_case(m_player_data.login) ==
											utils::string::lower_case(other_data.login)
									);
								}

								// Skip the current bridge
								return false;
							}
						);

						// Check that the bridge is found
						if (opt.has_value() == true)
						{
							// Get pointer to the bridge
							Bridge* bridge = opt.value();

							// Lock the bridge
							std::lock_guard<std::recursive_mutex> lock(bridge->get_lock());
							
							// Send the login packet to the server
							bridge->send_packet_gate( bridge->player().login_packet );
						}
					}
				}
				break;
		}
		
		// Check that the network bridge is closed
		if ( m_client_ctx.connected == false && m_server_ctx.connected == false )
		{
			// Remove the bridge from the common list of bridges
			m_server.remove_bridge(this);
		}
	}

	// Send a packet to the network bridge side
	bool Bridge::send_packet(endpoint& ctx, const IPacket& packet)
	{
		// Check that the socket is disconnected
		if (ctx.connected == false)
		{
			// Socket is disconnected, cancel packet sending
			return false;
		}

		// Write the packet to the output buffer and wait for the write event
		try
		{
			// Clear the output packet processing Buffer
			ctx.out_buf->clear();

			// Get packet size
			const std::size_t packet_size = packet.size();

			// Check that the packet fits into the output network buffer
			if (packet_size > ctx.out_buf->size())
			{
				// Write a message to log
				Logger::Instance().log(
					"Can't write a packet (ID: %u) into output processing buffer: The packet is too large (%u bytes)",
					packet.id(),
					packet_size
				);

				// The packet is too large 
				return false;
			}

			// Check that the packet fits into the output processing buffer
			if (ctx.send_buf->can_write_bytes(packet_size) == false)
			{
				// Write a message to log
				Logger::Instance().log(
					"Can't write a packet (ID: %u) into output network buffer: "
						"Not enough free space in the output network buffer for packet (%u bytes) ",
					packet.id(),
					packet_size
				);

				// Not enough free space in the output network buffer
				return false;
			}

			// Write the packet to the output processing buffer
			const std::size_t written = packet.write((*ctx.out_buf));

			// Check if packet encryption is enabled
			if (m_player_data.comm_encrypt == true)
			{
				// Encrypt the packet
				ctx.out_buf->apply(
					6,
					written,
					[&](char* data, std::size_t size)
					{
						common::CommEncrypt::encrypt_Noise(ctx.enc_send_key, data, size);
						common::CommEncrypt::encrypt_B(data, size, m_player_data.session_key, m_player_data.session_key_length, true);
					}
				);
			}

			// The number of bytes to copy
			std::size_t counter = written;

			// Temporary buffer
			char tmp[common::max_packet_size];

			// Copy the packet from the output packet processing buffer to the output network buffer
			while (counter != 0)
			{
				// Determine the minimum size of the data block
				std::size_t block_size = min(counter, sizeof(tmp));

				// Read a block of data from the output processing buffer
				ctx.out_buf->read(tmp, block_size);

				// Write a block of data to the output network buffer
				ctx.send_buf->write(tmp, block_size);

				// Reduce the number of packet bytes that are left to copy
				counter -= block_size;
			}

			// Wait for a data write event
			const int ret = event_add(ctx.write_event, nullptr);

			// Check that event is added to the event loop
			if (ret == -1)
			{
				// Write a message to log
				Logger::Instance().log("Failed to add socket write event while sending packet!");

				// Error
				return false;
			}

			// Packet will be sent
			return true;
		}
		catch (const linear_buffer_exception& e)
		{
			Logger::Instance().log("Exception 'linear_buffer_exception' is occurred on send packet procedure: %s", e.what());
		}
		catch (const ring_buffer_exception& e)
		{
			Logger::Instance().log("Exception 'ring_buffer_exception' is occurred on send packet procedure: %s", e.what());
		}
		catch (const std::exception& e)
		{
			Logger::Instance().log("Exception 'std::exception' is occurred on send packet procedure: %s", e.what());
		}
		catch (...)
		{
			Logger::Instance().log("Exception '...' is occurred on send packet procedure");
		}

		return false;
	}
}