#include "Server.h"
#include "Bridge.h"
#include "BridgeList.h"
#include "Utils.h"
#include "Logger.h"

#include <Windows.h>

#include <algorithm>
#include <functional>
#include <chrono>

#include "ChapStringPacketHandler.h"
#include "LoginResultPacketHandler.h"
#include "EnterMapPacketHandler.h"
#include "SetStallSuccessPacketHandler.h"
#include "SetStallDelPacketHandler.h"
#include "PingRequestPacketHandler.h"

#include "LoginPacketHandler.h"
#include "CreatePinPacketHandler.h"
#include "UpdatePinPacketHandler.h"
#include "DisconnectPacketHandler.h"
#include "SetStallClosePacketHandler.h"
#include "SetStallStartPacketHandler.h"
#include "PersonalMessagePacketHandler.h"
#include "FriendInvitePacketHandler.h"
#include "TeamInvitePacketHandler.h"
#include "TalkSessionCreatePacketHandler.h"

#include "HelpConsoleCommand.h"
#include "DisconnectConsoleCommand.h"
#include "StopServerConsoleCommand.h"
#include "NoticeConsoleCommand.h"
#include "StatConsoleCommand.h"

namespace pkodev
{
	// Class instance counter
	std::size_t Server::instance_counter = 0;

	// Packet handlers list output flag
	std::atomic_bool Server::worker::handlers_log = false;


	// Server initializer constructor
	Server::initializer::initializer(std::function<void()> init_,
		std::function<void()> destroy_) :
		init(std::move(init_)),
		destroy(std::move(destroy_)),
		initialized(false)
	{

	}

	// Server initializer move constructor
	Server::initializer::initializer(initializer&& other) noexcept :
		init(std::move(other.init)),
		destroy(std::move(other.destroy)),
		initialized(other.initialized)
	{

	}

	// Server initializer copy constructor
	Server::initializer::initializer(const initializer& other) :
		init(other.init),
		destroy(other.destroy),
		initialized(other.initialized)
	{

	}



	// Server worker constructor
	Server::worker::worker(event_base* base, std::thread&& th) :
		evbase(base), 
		th(std::move(th)), 
		event_count(0),
		max_time(0)
	{
		// Create a list of network packets handlers
		handlers = std::make_unique<PacketHandlerStorage>();

		// S -> C
		handlers->add_handler(std::make_unique<ChapStringPacketHandler>());
		handlers->add_handler(std::make_unique<LoginResultPacketHandler>());
		handlers->add_handler(std::make_unique<EnterMapPacketHandler>());
		handlers->add_handler(std::make_unique<SetStallSuccessPacketHandler>());
		handlers->add_handler(std::make_unique<SetStallDelPacketHandler>());
		handlers->add_handler(std::make_unique<PingRequestPacketHandler>());
		
		// C -> S
		handlers->add_handler(std::make_unique<LoginPacketHandler>());
		handlers->add_handler(std::make_unique<CreatePinPacketHandler>());
		handlers->add_handler(std::make_unique<UpdatePinPacketHandler>());
		handlers->add_handler(std::make_unique<DisconnectPacketHandler>());
		handlers->add_handler(std::make_unique<SetStallClosePacketHandler>());
		handlers->add_handler(std::make_unique<PersonalMessagePacketHandler>());
		handlers->add_handler(std::make_unique<FriendInvitePacketHandler>());
		handlers->add_handler(std::make_unique<TeamInvitePacketHandler>());
		handlers->add_handler(std::make_unique<TalkSessionCreatePacketHandler>());
		handlers->add_handler(std::make_unique<SetStallStartPacketHandler>());

		// Print the list of registered handlers to the log
		if (handlers_log.load() == false)
		{
			// Raise the flag
			handlers_log = true;

			// Print the list of handlers
			{
				// Handler related data
				struct handler_info
				{
					// Fields
					unsigned short int id;    // Handler ID
					std::string name;         // Handler name

					// Constructor
					handler_info(unsigned short int id_, const std::string& name_) :
						id(id_),
						name(name_)
					{

					}
				};

				// Packet IDs intervals list
				std::vector< std::pair<unsigned short int, unsigned short int> > handler_intervals;

				// Fill in the list of intervals
				{
					// Game.exe -> GameServer.exe
					handler_intervals.push_back(
						std::make_pair((common::CMD_CM_BASE + 1), (common::CMD_CM_BASE + common::CMD_INTERVAL))
					);

					// GameServer.exe -> Game.exe
					handler_intervals.push_back(
						std::make_pair((common::CMD_MC_BASE + 1), (common::CMD_MC_BASE + common::CMD_INTERVAL))
					);

					// GroupServer.exe -> Game.exe
					handler_intervals.push_back(
						std::make_pair((common::CMD_PC_BASE + 1), (common::CMD_PC_BASE + common::CMD_INTERVAL))
					);

					// Game.exe -> GroupServer.exe
					handler_intervals.push_back(
						std::make_pair((common::CMD_CP_BASE + 1), (common::CMD_CP_BASE + common::CMD_INTERVAL))
					);
				}

				// Handlers lists
				std::vector<handler_info> client_handlers;   // From Game.exe
				std::vector<handler_info> server_handlers;   // From GateServer.exe

				// Build a list of handlers
				for (const auto& interval : handler_intervals)
				{
					// Walking through the interval
					for (unsigned short int i = interval.first; i < interval.second; ++i)
					{
						// Check that the handler with the given ID is registered
						if (handlers->check_handler(i) == true)
						{
							// Get a handler reference
							const handler_ptr_t& handler = handlers->get_handler(i);

							// Check the transfer direction
							switch (handler->direction())
							{
								// Game.exe -> GateServer.exe
								case packet_direction_t::cs:
									{
										client_handlers.push_back({ i, handler->name() });
									}
									break;

								// GateServer.exe -> Game.exe
								case packet_direction_t::sc:
									{
										server_handlers.push_back({ i, handler->name() });
									}
									break;
							}
						}
					}
				}

				// Condition for sorting handlers by ID
				auto sort_cond = [](const handler_info& info1, const handler_info& info2)
				{
					return (info1.id < info2.id);
				};

				// Sort lists by ID
				std::sort(client_handlers.begin(), client_handlers.end(), sort_cond);
				std::sort(server_handlers.begin(), server_handlers.end(), sort_cond);

				// Print packet handlers from Game.exe to the log
				{
					// Write a log
					Logger::Instance().log("Registered (%u) Game.exe packet handlers:", client_handlers.size());

					// Walking through the list
					for (const auto& info : client_handlers)
					{
						// Print a handler . . .
						Logger::Instance().log("* Packet ID %u (%04X): %s.", info.id, info.id, info.name.c_str());
					}
				}

				// Print packet handlers from GateServer.exe to the log
				{
					// Write a log
					Logger::Instance().log("Registered (%u) GateServer.exe packet handlers:", server_handlers.size());

					// Walking through the list
					for (const auto& info : server_handlers)
					{
						// Print a handler . . .
						Logger::Instance().log("* Packet ID %u (%04X): %s.", info.id, info.id, info.name.c_str());
					}
				}
			}
		}
	}

	// Server worker move constructor
	Server::worker::worker(worker&& w) noexcept:
		evbase(w.evbase), 
		th(std::move(w.th)),
		event_count(w.event_count.load()),
		max_time(w.max_time.load()),
		handlers(std::move(w.handlers))
	{

	}
	

	// Dummy function for the server initializer 
	static void _nop() {}

	// Server constructor
	Server::Server(const settings_t& settings_) :
		m_cfg(settings_),
		m_running(false), 
		m_startup_time({}),
		m_workers_ready(false), m_workers_stop(false),
		m_listener(nullptr)
	{
		// Check the number of instances of the class
		if (Server::instance_counter++ == 0)
		{
			// Make libevent threadsafe
			const int ret = evthread_use_windows_threads();

			// Check that evthread_use_windows_threads() failed
			if (ret == -1)
			{
				// Write a message to the log
				Logger::Instance().log("Warning: evthread_use_windows_threads() function failed!");
			}
		}

		// Fill out the resource initialization list
		{
			// Final release of resources
			m_inits.push_back(
				{
					_nop,
					std::bind(std::mem_fn(&Server::final_cleanup), this)
				}
			);

			// Load WinSock library
			m_inits.push_back(
				{
					std::bind(std::mem_fn(&Server::init_winsock), this),
					std::bind(std::mem_fn(&Server::destroy_winsock), this)
				}
			);
			
			// Memory allocation for clients
			m_inits.push_back(
				{
					std::bind(std::mem_fn(&Server::init_bridge_pool), this),
					_nop
				}
			);

			// Initialization of worker threads
			m_inits.push_back(
				{
					std::bind(std::mem_fn(&Server::init_workers), this),
					std::bind(std::mem_fn(&Server::destroy_workers), this)
				}
			);

			// Memory allocation for clients
			m_inits.push_back(
				{
					_nop,
					std::bind(std::mem_fn(&Server::destroy_bridge_pool), this)
				}
			);

			// Initialize listening for incoming connections
			m_inits.push_back(
				{
					std::bind(std::mem_fn(&Server::init_listener), this),
					std::bind(std::mem_fn(&Server::destroy_listener), this)
				}
			);

			// Initial release of resources
			m_inits.push_back(
				{
					_nop,
					std::bind(std::mem_fn(&Server::initial_cleanup), this)
				}
			);
		}

		// Create console commands
		m_console_commands.emplace("stop", std::make_unique<StopServerConsoleCommand>());
		m_console_commands.emplace("close", std::make_unique<StopServerConsoleCommand>("close"));
		m_console_commands.emplace("disconnect", std::make_unique<DisconnectConsoleCommand>());
		m_console_commands.emplace("help", std::make_unique<HelpConsoleCommand>());
		m_console_commands.emplace("notice", std::make_unique<NoticeConsoleCommand>());
		m_console_commands.emplace("stat", std::make_unique<StatConsoleCommand>());
	}

	// Server destructor
	Server::~Server()
	{
		// Stop the server
		if (m_running == true)
		{
			stop();
		}
		
		// Decrease the class instances counter
		--Server::instance_counter;
	}

	// Initial release of resources
	void Server::initial_cleanup()
	{
		// Write a log
		Logger::Instance().log("Initial cleanup . . .");


		// Write a log
		Logger::Instance().log("Initial cleanup done!");
	}

	// Final release of resources
	void Server::final_cleanup()
	{
		// Write a log
		Logger::Instance().log("Final cleanup . . .");

		// Delete list of IP addresses
		m_ip_book.clear();

		// Clear the lists of network bridges
		m_connected_bridges.clear();
		m_offline_stall_bridges.clear();

		// Write a log
		Logger::Instance().log("Final cleanup done!");
	}

	// Load WinSock library
	void Server::init_winsock()
	{
		// Check the number of server instances
		if (Server::instance_counter == 1)
		{
			// Write a log
			Logger::Instance().log("Loading Winsock . . .");

			// Structure with socket implementation data
			WSAData wsaData;
			std::memset(reinterpret_cast<void*>(&wsaData), 0x00, sizeof(wsaData));

			// Load WinSock 2.2 library
			const int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);

			// Check the result
			if (ret != 0)
			{
				// Raise an exception
				throw server_exception(
					std::string("WSAStartup() failed with error code " + std::to_string(WSAGetLastError()) + "!")
				);
			}

			// Write a log
			Logger::Instance().log("Winsock loaded!");
		}
	}

	// Release WinSock library
	void Server::destroy_winsock()
	{
		// Check the number of server instances
		if (Server::instance_counter == 1)
		{
			// Write a log
			Logger::Instance().log("Clearing Winsock . . .");

			// Release WinSock 2.2 library
			const int ret = WSACleanup();

			// Check the result
			if (ret != 0)
			{
				// Write a log
				Logger::Instance().log("WSACleanup() failed with error code %d!", WSAGetLastError());
			}
			else
			{
				// Write a log
				Logger::Instance().log("Winsock cleared!");
			}
		}
	}

	// Network clients memory allocation 
	void Server::init_bridge_pool()
	{
		// Write a log
		Logger::Instance().log("Allocating memory for %u clients . . .", m_cfg.max_player);

		// Create a pool of network bridges
		try
		{
			// Allocate memory for max_player clients
			m_bridge_pool = std::make_unique<CObjectPool>(
				std::make_unique<BridgeMaker>(*this),
				m_cfg.max_player
			);
		}
		catch (const std::bad_alloc& e)
		{
			// Write a log
			Logger::Instance().log("Can't allocate memory: %s", e.what());

			// Not enough memory to create network bridge pool
			throw server_exception(
				std::string("Can't create client pool: " + std::string(e.what()))
			);
		}

		// Write a log
		Logger::Instance().log("Memory successfully allocated!");
	}

	// Network clients memory deallocation 
	void Server::destroy_bridge_pool()
	{
		// Write a log
		Logger::Instance().log("Releasing client memory . . .");

		// Return network bridges to the pool
		m_connected_bridges.for_each(
			[&](Bridge& bridge)
			{
				m_bridge_pool->release(&bridge);
				return false;
			}
		);

		// Clear the list of connected network bridges
		m_connected_bridges.clear();

		// Delete the bridges pool
		m_bridge_pool.reset();

		// Write a log
		Logger::Instance().log("Memory released!");
	}

	// Start worker threads
	void Server::init_workers()
	{
		// Reset workers flags
		m_workers_ready = false;
		m_workers_stop = false;

		// Get the amount of available worker threads
		const std::size_t count = static_cast<std::size_t>(
			max(2, std::thread::hardware_concurrency())
		);

		// Write a log
		Logger::Instance().log("Starting %u worker threads . . .", count);

		// Allocate some memory for the workers list
		m_workers.reserve(count);

		// Start all worker threads
		for (std::size_t i = 0; i < count; ++i)
		{
			// Create an event base
			event_base* evbase = event_base_new();

			// Check that the event base has been created
			if (evbase != nullptr)
			{
				// Start a thread for the new worker
				std::thread th(
					std::bind(
						std::mem_fn(&pkodev::Server::work),
						this
					)
				);
				
				// Get the thread ID
				std::thread::id thread_id = th.get_id();

				// Initialize the worker and put it in the list of workers
				m_workers.push_back({evbase, std::move(th)});
				
				// Write a log
				Logger::Instance().log("Worker thread %u/%u (ID: %04X) successfully started!", (i + 1), count, thread_id);
			}
			else
			{
				// Failed to create event base
				m_workers_stop = true;

				// Stop workers that have already started
				for (auto& w : m_workers)
				{
					// Attach the worker's thread to the main thread
					if (w.th.joinable() == true)
					{
						w.th.join();
					}

					// Free the worker's event base
					event_base_free(w.evbase);
				}

				// Clear the list of workers
				m_workers.clear();

				// Write a log
				Logger::Instance().log("Failed to create worker thread %u/%u!", (i + 1), count);

				// Raise the server exception
				throw server_exception("Failed to create worker threads!");;
			}
		}

		// Workers started
		m_workers_ready = true;

		// Write a log
		Logger::Instance().log("All worker threads are successfully started!");
	}

	// Stop worker threads
	void Server::destroy_workers()
	{
		// Write a log
		Logger::Instance().log("Stopping all worker threads . . .");

		// Stop all workers
		for (auto& w : m_workers)
		{
			// Stop the worker's event loop
			event_base_loopbreak(w.evbase);

			// Get the worker's ID
			std::thread::id thread_id = w.th.get_id();

			// Attach the worker's thread to the main thread
			if (w.th.joinable() == true)
			{
				w.th.join();
			}	
			
			// Free the worker's event base
			event_base_free(w.evbase);

			// Write a log
			Logger::Instance().log("Worker thread (ID: %04X) successfully stopped!", thread_id);
		}

		// Clear the list of workers
		m_workers.clear();

		// Reset the workers ready flag
		m_workers_ready = false;

		// Write a log
		Logger::Instance().log("All worker threads are successfully stopped!");
	}

	// Start listen for incoming connections
	void Server::init_listener()
	{
		// Write a log
		Logger::Instance().log("The process of listening for incoming connections on address (%s:%u) starts . . .", m_cfg.game_host.c_str(), m_cfg.game_port);

		// The local address structure
		sockaddr_in local;
		std::memset(reinterpret_cast<void*>(&local), 0x00, sizeof(local));

		// Fill in the local address structure
		local.sin_family = AF_INET;
		local.sin_port = htons(m_cfg.game_port);
		const int ret = inet_pton(AF_INET, m_cfg.game_host.c_str(), &local.sin_addr);

		// Check the result of the inet_pton() function
		if (ret == SOCKET_ERROR)
		{
			// Write a log
			Logger::Instance().log("inet_pton() failed! Error code %d: ", WSAGetLastError());

			// Raise the server exception
			throw server_exception("Failed to convert server IP address into its numeric binary form (inet_pton() error).");
		}

		// Get a free worker
		worker& w = get_min_worker();

		// Start the process of accepting incoming connections
		m_listener = evconnlistener_new_bind(
			w.evbase,
			[](evconnlistener* listener, evutil_socket_t fd,
				sockaddr* address, int length, void* ctx) noexcept
			{
				try
				{
					// Accept the incoming connection
					const bool ret = reinterpret_cast<Server*>(ctx)->handle_accept(fd, address);

					// Check the result
					if (ret == false)
					{
						// Close the connection
						evutil_closesocket(fd);
					}
				}
				catch (...)
				{
					// Error, close the connection
					evutil_closesocket(fd);
				}
			},
			this,
			LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
			-1,
			reinterpret_cast<sockaddr*>(&local),
			sizeof(local)
		);

		// Check that the listening process is running
		if (m_listener == nullptr)
		{
			// Write a log
			Logger::Instance().log("evconnlistener_new_bind() failed! Is the port already in use?");

			// Raise the server exception
			throw server_exception(
				std::string("Failed to start listening on address (" + m_cfg.game_host
					+ ":" + std::to_string(m_cfg.game_port) + ")! Is the port already in use?")
			);
		}

		// Increase the number of tasks for the worker
		++w.event_count;

		// Write a log
		Logger::Instance().log("Ready to accept incoming connections!");
	}

	// Stop listen for incoming connections
	void Server::destroy_listener()
	{
		// Write a log
		Logger::Instance().log("Stop receiving incoming connections . . .");

		// Stop listen
		evconnlistener_free(m_listener); 
		m_listener = nullptr;

		// Write a log
		Logger::Instance().log("Receiving incoming connections stopped!");
	}

	// Start the server
	void Server::run()
	{
		// Write a log
		Logger::Instance().log("Server starts . . .");

		// Check if the server is already running
		if (m_running == true)
		{
			// Write a log
			Logger::Instance().log("Error, the server is already running!");

			// Raise the server exception
			throw server_exception("Server is already running!");
		}

		// Starting the server . . .
		try
		{
			// Loop through the server initialization list
			for (auto it = m_inits.begin(); it != m_inits.end(); ++it)
			{
				// Perform initialization
				it->init();

				// Raise the initialization flag
				it->initialized = true;
			}
		}
		catch (const std::exception& e)
		{
			// Rollback initialization . . .
			for (auto it = m_inits.rbegin(); it != m_inits.rend(); ++it)
			{
				// Check that the initialization flag is raised
				if (it->initialized == true)
				{
					// Free up resources
					it->destroy();

					// Reset the flag
					it->initialized = false;
				}
			}

			// Raise the server exception
			throw server_exception(e.what());
		}

		// The server is started
		m_running = true;

		// Get startup time
		m_startup_time = std::chrono::system_clock::now();

		// Write a log
		Logger::Instance().log("Server started!");
	}

	// Stop the server
	void Server::stop()
	{
		// Write a log
		Logger::Instance().log("Stopping server . . .");

		// Check if the server is running
		if (m_running == false)
		{
			// Write a log
			Logger::Instance().log("The server is already stopped!");

			// The server is already stopped
			return;
		}

		// Free resources
		for (auto it = m_inits.rbegin(); it != m_inits.rend(); ++it)
		{
			// Check that the initialization flag is raised
			if (it->initialized == true)
			{
				// Free up resources
				it->destroy();

				// Reset the flag
				it->initialized = false;
			}
		}

		// The server is stopped
		m_running = false;

		// Write a log
		Logger::Instance().log("The server stopped!");
	}

	// Execute a command
	void Server::execute_cmd(const std::string& cmd)
	{
		/*
			/stop - Stop the server
			/stat - Print statistics
			/disconnect all|offline - Disconnect clients
			/notice message - Send a message to clients in system chat channel
			/help - Show available commands
		*/

		// Trim whitespaces
		std::string raw = utils::string::trim(cmd);

		// Check that given string is not empty
		if (raw.empty() == true)
		{
			// The command is empty
			return;
		}

		// Check that command has '/' symbol at the beginning
		if (raw.front() != '/')
		{
			// The wrong command format
			return;
		}

		// Remove the '/' symbol
		raw.erase(0, 1);

		// Split the command line with spaces
		std::vector<std::string> params;
		pkodev::utils::string::split(raw, params, ' ');

		// Check substrings number
		if (params.size() > 0)
		{
			// Get command name
			const std::string command = pkodev::utils::string::lower_case(params[0]);

			// Remove command 
			params.erase(params.begin());

			// Search the command handler
			const auto it = m_console_commands.find(command);

			// Check that command is found
			if (it != m_console_commands.end())
			{
				// Execute the command
				const bool ret = it->second->execute(params, *this);

				// Check the result
				if (ret == false)
				{
					std::cout << "Command failed!" << std::endl;
				}
			}
			else
			{
				// Unknown command
				std::cout << "'/" << command << "': Unknown command!" << std::endl;
			}
		}
	}

	// Accept incoming connection
	bool Server::handle_accept(evutil_socket_t fd, sockaddr* address)
	{
		// Handling an incoming connection
		try
		{
			// Check if there are free slots on the server
			if (m_bridge_pool->is_empty() == true)
			{
				// There are no free slots left on the server
				return false;
			}

			// Get the IP address and port of the client
			const std::string ip_address = utils::network::get_ip_address(address);
			const unsigned short int port = utils::network::get_port(address);

			// Check the IP address and port of the client
			if ( (ip_address.empty() == true) || (port == 0) )
			{
				// Invalid client address
				return false;
			}

			// Check that the limit of connections from one IP address has not been exceeded
			if ( (m_cfg.max_clients_per_ip != 0) &&
					(m_ip_book.get_ip_count(ip_address) >= m_cfg.max_clients_per_ip) )
			{
				// Connection limit from one IP address exceeded
				return false;
			}

			// Check connection time from one IP address
			if ( (m_cfg.connection_interval != 0) &&
					(m_ip_book.is_time_expired(ip_address, m_cfg.connection_interval) == false) )
			{
				// Too frequent connections from the same IP address
				return false;
			}

			// Look for a worker to which we can attach a client
			worker& w = get_min_worker();

			// Lambda to automatically return the bridge to the pool on an error
			auto releaser = [&](Bridge* bridge) noexcept
			{
				// Catch and handle all exceptions
				try
				{
					// Return the bridge to the pool
					m_bridge_pool->release(bridge);
				}
				catch (...)
				{
					// Write a log
					Logger::Instance().log("Caught an exception in bridge releaser on client accept!");
				}
			};
			
			// Take a bridge from the pool
			std::unique_ptr<Bridge, decltype(releaser)> bridge_ptr(
				dynamic_cast<Bridge*>(m_bridge_pool->acquire()), 
				releaser
			);

			// Associate the network bridge with Game.exe (client) and set the address of GateServer.exe
			bridge_ptr->build(
				w,
				fd,
				{ ip_address, port }, 
				{ m_cfg.gate_host, m_cfg.gate_port }
			);

			// Start connecting to GateServer.exe
			bridge_ptr->connect();
			
			// Add a bridge to the list
			const bool ret = m_connected_bridges.add(bridge_ptr.get());

			// Check the result
			if (ret == true)
			{
				// Add client IP address to address list
				m_ip_book.register_ip(bridge_ptr->game_address().ip);
			}
			else
			{
				// Failed to add the bridge to the list?
				Logger::Instance().log("Server::add_bridge(): Failed to add a bridge to the list!");
				return false;
			}

			// Increase the task counter for the current worker
			++w.event_count;

			// Do not delete the bridge, just release the pointer
			bridge_ptr.release();

			// Connection successfully accepted
			return true;
		}
		catch (const bridge_exception& e)
		{
			// Write a log
			Logger::Instance().log("Exception 'bridge_exception' is occurred on client accept: %s", e.what());
		}
		catch (const object_pool_exception& e)
		{
			// Write a log
			Logger::Instance().log("Exception 'object_pool_exception' is occurred on client accept: %s", e.what());
		}
		catch (const std::exception& e)
		{
			// Write a log
			Logger::Instance().log("Exception 'std::exception' is occurred on client accept: %s", e.what());
		}
		catch (...)
		{
			// Write a log
			Logger::Instance().log("Exception '...' is occurred on client accept");
		}

		// An error has occurred while accepting the connection
		return false;
	}

	// Find the worker with the least number of tasks
	Server::worker& Server::get_min_worker()
	{
		// Pointer to the first element in the list of workers
		auto min_it = m_workers.begin();

		// Looking for the worker with the least number of tasks
		for (auto it = (min_it + 1); it != m_workers.end(); ++it)
		{
			// Compare the number of tasks
			if (it->event_count.load() < min_it->event_count.load())
			{
				min_it = it;
			}
		}

		return *min_it;
	}

	// The worker's procedure
	void Server::work()
	{
		// Wait until all worker threads start
		while (m_workers_ready == false)
		{
			// Check that all worker threads need to be stopped
			if (m_workers_stop == true)
			{
				// Exit the procedure
				return;
			}

			// Waiting for 10ms
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		// Get the ID of the current thread
		std::thread::id this_id = std::this_thread::get_id();

		// Look for a worker that belongs to this thread
		worker* w = nullptr;
		for (auto it = m_workers.begin(); it != m_workers.end(); ++it)
		{
			// Compare thread IDs
			if ( it->th.get_id() == this_id )
			{
				// Worker found
				w = &(*it);
				break;
			}
		}

		// Execute the event loop
		if ( w != nullptr )
		{
			while (event_base_got_break(w->evbase) == false)
			{
				// Start measure operation time
				auto start = std::chrono::steady_clock::now();

				// Handle events
				event_base_loop(w->evbase, EVLOOP_NONBLOCK);

				// Stop measure operation time
				auto end = std::chrono::steady_clock::now();

				// Calculate time difference in microseconds
				unsigned long long diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

				// Get max operation time
				unsigned long long max_time = w->max_time.load();
				w->max_time = (diff > max_time) ? diff : max_time;

				// Wait 1ms
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	}

	// Remove a network bridge from the list
	void Server::remove_bridge(Bridge* bridge)
	{
		// Remove a bridge from the list
		const bool ret = m_connected_bridges.remove(bridge);

		// Check the result
		if (ret == true)
		{
			// Remove an IP address from the list
			m_ip_book.unregister_ip(
				bridge->game_address().ip,
				m_cfg.connection_interval
			);

			// Return the bridge to the pool
			m_bridge_pool->release(bridge);

			// Get the current thread ID
			std::thread::id this_id = std::this_thread::get_id();

			// Decrease the number of tasks
			for (auto it = m_workers.begin(); it != m_workers.end(); ++it)
			{
				if (it->th.get_id() == this_id)
				{
					--it->event_count;
					break;
				}
			}
		}
		else
		{
			// Failed to remove the bridge from the list?
			Logger::Instance().log("Server::remove_bridge(): Bridge is not found in the list!");
		}
	}

	// Get a list of workers information
	const std::vector<worker_info_t> Server::make_worker_info() const
	{
		// The list of workers
		std::vector<worker_info_t> info;

		// Make the list . . .
		for (auto it = m_workers.cbegin(); it != m_workers.cend(); ++it)
		{
			info.push_back({ it->th.get_id(), it->event_count.load(), it->max_time.load() });
		}

		return info;
	}
}