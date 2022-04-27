#pragma once
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <stdexcept>
#include <functional>

#include <event.h>
#include <event2/event.h>
#include <event2/thread.h>
#include <event2/listener.h>

#include "ObjectPool.h"
#include "PacketHandlerStorage.h"
#include "IpAddressBook.h"
#include "BridgeList.h"
#include "ConsoleCommand.h"

namespace pkodev
{
	// Define some useful classes
	class Bridge;
	class BridgeList;

	// Define the type of the list of maps on which the system of offline stalls works
	typedef std::vector<std::string> map_list_t;

	// Define the type of the list of console commands
	typedef std::map< std::string, std::unique_ptr<IConsoleCommand> > console_command_list_t;

	// Stall server exception
	class server_exception final : public std::runtime_error
	{
		public:

			// Constructor for const char *
			server_exception(const char* what) :
				std::runtime_error(what) { }

			// Constructor for std::string
			server_exception(const std::string& what) :
				std::runtime_error(what) { }
	};

	// Structure with the server settings
	struct settings_t final
	{
		// Connection settings for StallServer.exe
		std::string game_host;                  // Address Game.exe -> StallServer.exe
		unsigned short int game_port;           // Port  Game.exe -> StallServer.exe
		unsigned short int max_player;          // The maximum number of clients that can simultaneously connect to the server
		unsigned short int max_clients_per_ip;  // Maximum number of clients with the same IP address
		unsigned int connection_interval;       // Time interval between connections from the same IP address

		// Connection settings for GateServer.exe
		std::string gate_host;         // Address StallServer.exe -> GateServer.exe
		unsigned short int gate_port;  // Port  StallServer.exe -> GateServer.exe
		bool gate_ip_mod;              // Is the mod 'pkodev.mod.stallip' installed for GateServer.exe?

		// The list of maps on which the system of offline stalls works
		map_list_t maps;

		// Maximum number of stalls from 1 IP address
		unsigned short int max_stalls_per_ip;

		// Maximum trading time in an offline stall (in seconds)
		unsigned int max_offline_time;

		// Close an offline stall if it is empty
		bool close_stall_on_empty;

		// Constructor
		settings_t() :
			game_host(""), game_port(0), max_player(0), 
			max_clients_per_ip(0), connection_interval(0),
			gate_host(""), gate_port(0), gate_ip_mod(true),
			max_stalls_per_ip(0), max_offline_time(0), close_stall_on_empty(true)
		{
			maps.reserve(8);
		}
	};

	// Server class
	class Server final
	{
		public:

			// Constructor
			Server(const settings_t& settings_);

			// Copy constructor
			Server(const Server&) = delete;

			// Move constructor
			Server(Server&&) = delete;

			// Destructor
			~Server();

			// Copy assigment operator
			Server& operator=(const Server&) = delete;

			// Move assigment operator
			Server& operator=(Server&&) = delete;

			// Start the server
			void run();

			// Stop the server
			void stop();

			// Execute a command
			void execute_cmd(const std::string& cmd);

			// Get a reference to the server settings structure
			inline const settings_t& settings() const { return m_cfg; }

			// Is the server running?
			inline bool is_running() const { return m_running; }

			// Get a list of network bridges
			inline BridgeList& bridges() { return m_connected_bridges; }

			// Get a list of network bridges in offline stall mode
			inline BridgeList& offline_bridges() { return m_offline_stall_bridges; }

			// Get a list of console commands
			inline const console_command_list_t& console_commands() const { return m_console_commands; }

		private:

			friend class Bridge;

			// Server initializer structure
			struct initializer final
			{
				// Initialization function
				std::function<void()> init;

				// Resource release function
				std::function<void()> destroy;

				// Initialization flag
				bool initialized;

				// Constructor
				initializer(std::function<void()> init_, std::function<void()> destroy_);

				// Copy constructor
				initializer(initializer&& other) noexcept;

				// Move constructor
				initializer(const initializer& other);
			};

			// Server worker structure
			struct worker final
			{
				// Packet handlers list output flag
				static std::atomic_bool handlers_log;

				// Event base
				event_base* evbase;

				// Thread descriptor
				std::thread th;

				// Number of tasks
				std::atomic_int event_count;

				// Packet handlers
				std::shared_ptr<PacketHandlerStorage> handlers;


				// Constructor
				worker(event_base* base, std::thread&& th);

				// Move constructor
				worker(worker&& other) noexcept;

				// Copy constructor
				worker(const worker& other) = delete;
			};


			// Initial and final release of resources
			void initial_cleanup();
			void final_cleanup();

			// Load WinSock library
			void init_winsock();
			void destroy_winsock();
			
			// Memory allocation for clients
			void init_bridge_pool();
			void destroy_bridge_pool();

			// Initialization of worker threads
			void init_workers();
			void destroy_workers();

			// Initialize listening for incoming connections
			void init_listener();
			void destroy_listener();


			// Accept incoming connection
			bool handle_accept(evutil_socket_t fd, sockaddr* address);

			// The worker's procedure
			void work();

			// Find the worker with the least number of tasks
			worker& get_min_worker();

			// Add a network bridge to the list
			void add_bridge(Bridge* bridge);

			// Remove a network bridge from the list
			void remove_bridge(Bridge* bridge);


			// Class instance counter
			static std::size_t instance_counter;

			// List of initializers
			std::vector<initializer> m_inits;

			// Reference to the structure with server settings
			const settings_t& m_cfg;
			
			// Is the server running?
			bool m_running;

			// List of worker threads
			std::vector<worker> m_workers;

			// Are all worker threads running?
			std::atomic<bool> m_workers_ready;

			// Should worker threads be stopped?
			std::atomic<bool> m_workers_stop;

			// Object for listening to incoming connections
			evconnlistener* m_listener;

			// Network bridge object pool
			std::unique_ptr<CObjectPool> m_bridge_pool;

			// List of all connected network bridges
			BridgeList m_connected_bridges;

			// List of network bridges that are in offline stall mode
			BridgeList m_offline_stall_bridges;

			// List of connected IP addresses
			IpAddressBook m_ip_book;

			// Adminitstrator commands
			console_command_list_t m_console_commands;
	};
}