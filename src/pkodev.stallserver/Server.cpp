#include "Server.h"
#include "Bridge.h"
#include "BridgeList.h"
#include "Utils.h"
#include "Logger.h"

#include <Windows.h>

#include <algorithm>
#include <functional>
#include <ctime>

#include "ChapStringPacketHandler.h"
#include "LoginResultPacketHandler.h"
#include "EnterMapPacketHandler.h"
#include "SetStallSuccessPacketHandler.h"
#include "PingRequestPacketHandler.h"
#include "PersonalMessagePacketHandler.h"
#include "FriendInvitePacketHandler.h"
#include "TeamInvitePacketHandler.h"
#include "TalkSessionCreatePacketHandler.h"

#include "LoginPacketHandler.h"
#include "CreatePinPacketHandler.h"
#include "UpdatePinPacketHandler.h"
#include "DisconnectPacketHandler.h"
#include "SetStallClosePacketHandler.h"


namespace pkodev
{
	// Class instance counter
	std::size_t Server::instance_counter = 0;

	// Packet handlers list output flag
	bool Server::worker::handlers_log = false;


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
		out_lock(std::make_shared<std::recursive_mutex>()),
		data_lock(std::make_shared<std::recursive_mutex>())
	{
		// Create a list of network packets handlers
		handlers = std::make_shared<PacketHandlerStorage>();

		// S -> C
		handlers->add_handler(std::make_shared<ChapStringPacketHandler>());
		handlers->add_handler(std::make_shared<LoginResultPacketHandler>());
		handlers->add_handler(std::make_shared<EnterMapPacketHandler>());
		handlers->add_handler(std::make_shared<SetStallSuccessPacketHandler>());
		handlers->add_handler(std::make_shared<PingRequestPacketHandler>());
		
		// C -> S
		handlers->add_handler(std::make_shared<LoginPacketHandler>());
		handlers->add_handler(std::make_shared<CreatePinPacketHandler>());
		handlers->add_handler(std::make_shared<UpdatePinPacketHandler>());
		handlers->add_handler(std::make_shared<DisconnectPacketHandler>());
		handlers->add_handler(std::make_shared<SetStallClosePacketHandler>());
		handlers->add_handler(std::make_shared<PersonalMessagePacketHandler>());
		handlers->add_handler(std::make_shared<FriendInvitePacketHandler>());
		handlers->add_handler(std::make_shared<TeamInvitePacketHandler>());
		handlers->add_handler(std::make_shared<TalkSessionCreatePacketHandler>());

		// Print the list of registered handlers to the log
		if (handlers_log == false)
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
					Logger::Instance().log("Registered (%d) Game.exe packet handlers:", client_handlers.size());

					// Walking through the list
					for (const auto& info : client_handlers)
					{
						// Print a handler . . .
						Logger::Instance().log("* Packet ID %d (%04X): %s.", info.id, info.id, info.name.c_str());
					}
				}

				// Print packet handlers from GateServer.exe to the log
				{
					// Write a log
					Logger::Instance().log("Registered (%d) GateServer.exe packet handlers:", server_handlers.size());

					// Walking through the list
					for (const auto& info : server_handlers)
					{
						// Print a handler . . .
						Logger::Instance().log("* Packet ID %d (%04X): %s.", info.id, info.id, info.name.c_str());
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
		handlers(std::move(w.handlers)),
		out_lock(std::move(w.out_lock)),
		data_lock(std::move(w.data_lock))
	{

	}
	

	// Dummy function for the server initializer 
	static void _nop() {}

	// Server constructor
	Server::Server(const settings_t& settings_) :
		m_cfg(settings_),
		m_running(false), 
		m_workers_ready(false), m_workers_stop(false),
		m_listener(nullptr)
	{
		// Check the number of instances of the class
		if (Server::instance_counter == 0)
		{
			// Make libevent threadsafe
			int ret = evthread_use_windows_threads();

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
					std::bind(std::mem_fn(&Server::destroy_bridge_pool), this)
				}
			);

			// Initialization of worker threads
			m_inits.push_back(
				{
					std::bind(std::mem_fn(&Server::init_workers), this),
					std::bind(std::mem_fn(&Server::destroy_workers), this)
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

		// Increase the class instances counter
		Server::instance_counter++;
	}

	// Server destructor
	Server::~Server()
	{
		// Stop the server
		stop();

		// Decrease the class instances counter
		Server::instance_counter--;
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
			int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);

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
			int ret = WSACleanup();

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
		Logger::Instance().log("Allocating memory for %d clients . . .", m_cfg.max_player);

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
			[&](Bridge& bridge, bool& stop)
			{
				m_bridge_pool->release(&bridge);
			}
		);

		// Delete the pool
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
		std::size_t count = static_cast<std::size_t>(
			max(2, std::thread::hardware_concurrency())
		);

		// Write a log
		Logger::Instance().log("Starting %d worker threads . . .", count);

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
				Logger::Instance().log("Worker thread %d/%d (ID: %04X) successfully started!", (i + 1), count, thread_id);
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
				Logger::Instance().log("Failed to create worker thread %d/%d!", (i + 1), count);

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
		Logger::Instance().log("The process of listening for incoming connections on address (%s:%d) starts . . .", m_cfg.game_host.c_str(), m_cfg.game_port);

		// The local address structure
		sockaddr_in local;
		std::memset(reinterpret_cast<void*>(&local), 0x00, sizeof(local));

		// Fill in the local address structure
		local.sin_family = AF_INET;
		local.sin_port = htons(m_cfg.game_port);
		int ret = inet_pton(AF_INET, m_cfg.game_host.c_str(), &local.sin_addr);

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
					bool ret = reinterpret_cast<Server*>(ctx)->handle_accept(fd, address);

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
			Logger::Instance().log("Error, the server is already stopped!");

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
			std::string ip_address = utils::network::get_ip_address(address);
			unsigned short int port = utils::network::get_port(address);

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

			// Start connection to GateServer.exe
			bridge_ptr->connect();
			
			// The bridge is created, add it to the list of bridges
			add_bridge(bridge_ptr.release());

			// Increase the task counter for the current worker
			++w.event_count;

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
				event_base_loop(w->evbase, EVLOOP_NONBLOCK);
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	}

	// Add a network bridge to the list
	void Server::add_bridge(Bridge* bridge)
	{
		// Add a bridge to the list
		bool ret = m_connected_bridges.add(bridge);

		// Check the result
		if (ret == true)
		{
			// Add client IP address to address list
			m_ip_book.register_ip(bridge->game_address().ip);
		}
		else
		{
			// Failed to add the bridge to the list?
			Logger::Instance().log("Server::add_bridge(): Failed to add a bridge to the list!");
		}
	}

	// Remove a network bridge from the list
	void Server::remove_bridge(Bridge* bridge)
	{
		// Remove a bridge from the list
		bool ret = m_connected_bridges.remove(bridge);

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
		}
		else
		{
			// Failed to remove the bridge from the list?
			Logger::Instance().log("Server::remove_bridge(): Bridge is not found in the list!");
		}
	}
}