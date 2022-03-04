#pragma once
#include <vector>
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

namespace pkodev
{
	// ��������� ��� �������� �����
	class Bridge;
	class BridgeList;

	// ��������� ��� ������ ����, �� ������� �������� ������� ������� �������
	typedef std::vector<std::string> map_list_t;

	// ���������� ��� ������ �������
	class server_exception final : public std::runtime_error
	{
		public:

			// ����������� � const char *
			server_exception(const char* what) :
				std::runtime_error(what) { }

			// ����������� � std::string
			server_exception(const std::string& what) :
				std::runtime_error(what) { }
	};

	// ��������� � ����������� �������
	struct settings_t final
	{
		// ��������� ����������� � StallServer.exe
		std::string game_host;                  // ����� Game.exe -> StallServer.exe
		unsigned short int game_port;           // ����  Game.exe -> StallServer.exe
		unsigned short int max_player;          // ������������ ����� ��������, ������� ������������ ����� ������������ � �������
		unsigned short int max_clients_per_ip;  // ������������ ����� �������� � ���������� IP-�������
		unsigned int connection_interval;       // �������� ������� ����� ������������� � ������ IP-������

		// ��������� ����������� � GateServer.exe
		std::string gate_host;         // ����� StallServer.exe -> GateServer.exe
		unsigned short int gate_port;  // ����  StallServer.exe -> GateServer.exe

		// ������ ����, �� ������� �������� ������� ������� �������
		map_list_t maps;

		// ������������ ����� ������� � 1 IP-������
		unsigned short int max_stalls_per_ip;

		// ������������ ����� �������� � ������� ������ � ��������
		unsigned int max_offline_time;

		// �����������
		settings_t() :
			game_host(""), game_port(0), max_player(0), 
			max_clients_per_ip(0), connection_interval(0),
			gate_host(""), gate_port(0),
			max_stalls_per_ip(0), max_offline_time(0)
		{
			maps.reserve(8);
		}
	};

	// ����� �������
	class Server final
	{
		public:

			// �����������
			Server(const settings_t& settings_);

			// ����������� �����������
			Server(const Server&) = delete;

			// ����������� �����������
			Server(Server&&) = delete;

			// ����������
			~Server();

			// �������� ������������ ������������
			Server& operator=(const Server&) = delete;

			// �������� ������������ ������������
			Server& operator=(Server&&) = delete;

			// ��������� ������
			void run();

			// ���������� ������
			void stop();

			// �������� ������ �� ���������
			inline const settings_t& settings() const { return m_cfg; }

			// ������� �� ������
			inline bool is_running() const { return m_running; }

			// �������� ������ ������
			inline BridgeList& bridges() { return m_connected_bridges; }

			// �������� ������ ������ � ������ ������� ������
			inline BridgeList& offline_bridges() { return m_offline_stall_bridges; }


		private:

			friend class Bridge;

			// ��������� ��������������
			struct initializer final
			{
				// �������������
				std::function<void()> init;

				// ������������ ��������
				std::function<void()> destroy;

				// ���� �������������
				bool initialized;

				// �����������
				initializer(std::function<void()> init_, std::function<void()> destroy_);

				// ������������ �����������
				initializer(initializer&& other) noexcept;

				// ���������� �����������
				initializer(const initializer& other);
			};

			// ��������� ��������
			struct worker final
			{
				// ���� ������ ������ ������������
				static bool handlers_log;

				// ���� �������
				event_base* evbase;

				// �����
				std::thread th;

				// ����� �����
				std::atomic_int event_count;

				// ����������� �������
				std::shared_ptr<PacketHandlerStorage> handlers;

				// A lock for output and disconnection operations
				std::shared_ptr<std::recursive_mutex> out_lock;

				// A lock for player-related data
				std::shared_ptr<std::recursive_mutex> data_lock;


				// �����������
				worker(event_base* base, std::thread&& th);

				// ������������ �����������
				worker(worker&& other) noexcept;

				// ���������� �����������
				worker(const worker& other) = delete;
			};


			// ��������� � �������� ������������ ��������
			void initial_cleanup();
			void final_cleanup();

			// ��������� ���������� WinSock
			void init_winsock();
			void destroy_winsock();
			
			// ��������� ������ ��� �������
			void init_bridge_pool();
			void destroy_bridge_pool();

			// ������������� ������� �������
			void init_workers();
			void destroy_workers();

			// ������������� ������������� �������� ����������
			void init_listener();
			void destroy_listener();


			// ������� �������� ����������
			bool handle_accept(evutil_socket_t fd, sockaddr* address);

			// ���������, ������� ��������� �������
			void work();

			// ����� �������� � ���������� ������ �������
			worker& get_min_worker();

			// �������� ���� � ������
			void add_bridge(Bridge* bridge);

			// ������� ���� �� ������
			void remove_bridge(Bridge* bridge);


			// ������� ����������� ������
			static std::size_t instance_counter;


			// ������ ���������������
			std::vector<initializer> m_inits;


			// ������ �� ���������
			const settings_t& m_cfg;
			

			// ������� �� ������
			bool m_running;

			// ������ ������� �������
			std::vector<worker> m_workers;

			// �������� �� ��� ������� ������
			std::atomic<bool> m_workers_ready;

			// ����� �� ���������� ������� ������
			std::atomic<bool> m_workers_stop;

			// ������ ��� ������������� �������� ����������
			evconnlistener* m_listener;

			// ��� ������� ������
			std::unique_ptr<CObjectPool> m_bridge_pool;

			// ������ ���� ������������ ������
			BridgeList m_connected_bridges;

			// ������ ������, ������� ��������� � ������ ������� ������
			BridgeList m_offline_stall_bridges;

			// ������ ������������ IP �������
			IpAddressBook m_ip_book;
	};
}