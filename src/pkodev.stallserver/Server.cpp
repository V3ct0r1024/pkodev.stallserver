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
	// ������� ����������� ������
	std::size_t Server::instance_counter = 0;

	// ���� ������ ������ ������������
	bool Server::worker::handlers_log = false;


	// ����������� ��������������
	Server::initializer::initializer(std::function<void()> init_,
		std::function<void()> destroy_) :
		init(std::move(init_)),
		destroy(std::move(destroy_)),
		initialized(false)
	{

	}

	// ������������ ����������� ��������������
	Server::initializer::initializer(initializer&& other) noexcept :
		init(std::move(other.init)),
		destroy(std::move(other.destroy)),
		initialized(other.initialized)
	{

	}

	// ���������� ����������� ��������������
	Server::initializer::initializer(const initializer& other) :
		init(other.init),
		destroy(other.destroy),
		initialized(other.initialized)
	{

	}



	// ����������� ��������
	Server::worker::worker(event_base* base, std::thread&& th) :
		evbase(base), 
		th(std::move(th)), 
		event_count(0),
		out_lock(std::make_shared<std::recursive_mutex>()),
		data_lock(std::make_shared<std::recursive_mutex>())
	{
		// ������� ����������� �������
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

		// ������� ������ ������������
		if (handlers_log == false)
		{
			// ��������� ����
			handlers_log = true;

			// ���������� ������ ������������
			{
				// ����������
				struct handler_info
				{
					// ����
					unsigned short int id;    // ID �����������
					std::string name;         // �������� �����������

					// �����������
					handler_info(unsigned short int id_, const std::string& name_) :
						id(id_),
						name(name_)
					{

					}
				};

				// ������ ���������� �������
				std::vector< std::pair<unsigned short int, unsigned short int> > handler_intervals;

				// ��������� ������ ����������
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

				// ������ ������������
				std::vector<handler_info> client_handlers;   // �� Game.exe
				std::vector<handler_info> server_handlers;   // �� GateServer.exe

				// �������� ������ ������������
				for (const auto& interval : handler_intervals)
				{
					// �������� �� ���������
					for (unsigned short int i = interval.first; i < interval.second; ++i)
					{
						// ��������, ��� ���������� � ������ ID ���������������
						if (handlers->check_handler(i) == true)
						{
							// ������� ��������� �� ����������
							const handler_ptr_t& handler = handlers->get_handler(i);

							// �������� ����������� ��������
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

				// ������� ���������� ������������ �� ID
				auto sort_cond = [](const handler_info& info1, const handler_info& info2)
				{
					return (info1.id < info2.id);
				};

				// ����������� ������ �� ID
				std::sort(client_handlers.begin(), client_handlers.end(), sort_cond);
				std::sort(server_handlers.begin(), server_handlers.end(), sort_cond);

				// ������� ����������� ������� �� Game.exe
				{
					// ������� ���
					Logger::Instance().log("Registered (%d) Game.exe packet handlers:", client_handlers.size());

					// ���������� �� ������ ������������
					for (const auto& info : client_handlers)
					{
						// ������� ���
						Logger::Instance().log("* Packet ID %d (%04X): %s.", info.id, info.id, info.name.c_str());
					}
				}

				// ������� ����������� ������� �� GateServer.exe
				{
					// ������� ���
					Logger::Instance().log("Registered (%d) GateServer.exe packet handlers:", server_handlers.size());

					// ���������� �� ������ ������������
					for (const auto& info : server_handlers)
					{
						// ������� ���
						Logger::Instance().log("* Packet ID %d (%04X): %s.", info.id, info.id, info.name.c_str());
					}
				}
			}
		}
	}

	// ������������ ����������� ��������
	Server::worker::worker(worker&& w) noexcept:
		evbase(w.evbase), 
		th(std::move(w.th)),
		event_count(w.event_count.load()),
		handlers(std::move(w.handlers)),
		out_lock(std::move(w.out_lock)),
		data_lock(std::move(w.data_lock))
	{

	}
	
	
	// �������� ��� ������� ��������������
	static void _nop() {}

	// ����������� �������
	Server::Server(const settings_t& settings_) :
		m_cfg(settings_),
		m_running(false), 
		m_workers_ready(false), m_workers_stop(false),
		m_listener(nullptr)
	{
		// Check the number of instances
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

		// ���������� ������ ������������� ��������
		{
			// �������� ������������ ��������
			m_inits.push_back(
				{
					_nop,
					std::bind(std::mem_fn(&Server::final_cleanup), this)
				}
			);

			// �������� ���������� WinSock2
			m_inits.push_back(
				{
					std::bind(std::mem_fn(&Server::init_winsock), this),
					std::bind(std::mem_fn(&Server::destroy_winsock), this)
				}
			);

			// ��� ������� ������
			m_inits.push_back(
				{
					std::bind(std::mem_fn(&Server::init_bridge_pool), this),
					std::bind(std::mem_fn(&Server::destroy_bridge_pool), this)
				}
			);

			// ������� ������
			m_inits.push_back(
				{
					std::bind(std::mem_fn(&Server::init_workers), this),
					std::bind(std::mem_fn(&Server::destroy_workers), this)
				}
			);

			// ������������� �������� ����������
			m_inits.push_back(
				{
					std::bind(std::mem_fn(&Server::init_listener), this),
					std::bind(std::mem_fn(&Server::destroy_listener), this)
				}
			);

			// ��������� ������������ ��������
			m_inits.push_back(
				{
					_nop,
					std::bind(std::mem_fn(&Server::initial_cleanup), this)
				}
			);
		}

		// ����������� ������� �����������
		Server::instance_counter++;
	}

	// ���������� �������
	Server::~Server()
	{
		// ������������� ������
		stop();

		// ��������� ������� �����������
		Server::instance_counter--;
	}

	// ��������� ������������ ��������
	void Server::initial_cleanup()
	{
		// ������� ���
		Logger::Instance().log("Initial cleanup . . .");


		// ������� ���
		Logger::Instance().log("Initial cleanup done!");
	}

	// �������� ������������ ��������
	void Server::final_cleanup()
	{
		// ������� ���
		Logger::Instance().log("Final cleanup . . .");

		// ������� ������ IP-�������
		m_ip_book.clear();

		// ������� ������ ������
		m_connected_bridges.clear();
		m_offline_stall_bridges.clear();

		// ������� ���
		Logger::Instance().log("Final cleanup done!");
	}

	// ��������� ���������� WinSock
	void Server::init_winsock()
	{
		// �������� ������ ��������� �������
		if (Server::instance_counter == 1)
		{
			// ������� ���
			Logger::Instance().log("Loading Winsock . . .");

			// ��������� � ������� � ���������� �������
			WSAData wsaData;
			std::memset(reinterpret_cast<void*>(&wsaData), 0x00, sizeof(wsaData));

			// ��������� ���������� WinSock
			int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);

			// �������� ���������
			if (ret != 0)
			{
				// ����������� ����������
				throw server_exception(
					std::string("WSAStartup() failed with error code " + std::to_string(WSAGetLastError()) + "!")
				);
			}

			// ������� ���
			Logger::Instance().log("Winsock loaded!");
		}
	}

	// ��������� ���������� WinSock
	void Server::destroy_winsock()
	{
		// �������� ������ ��������� �������
		if (Server::instance_counter == 1)
		{
			// ������� ���
			Logger::Instance().log("Clearing Winsock . . .");

			// ��������� ���������� WinSock
			int ret = WSACleanup();

			// �������� ��� ������
			if (ret != 0)
			{
				// ������� ���
				Logger::Instance().log("WSACleanup() failed with error code %d!", WSAGetLastError());
			}
			else
			{
				// ������� ���
				Logger::Instance().log("Winsock cleared!");
			}
		}
	}

	// �������� ������ ��� �������
	void Server::init_bridge_pool()
	{
		// ������� ���
		Logger::Instance().log("Allocating memory for %d clients . . .", m_cfg.max_player);

		// ������� ��� ������� ������
		try
		{
			// �������� ������ ��� max_player �������
			m_bridge_pool = std::make_unique<CObjectPool>(
				std::make_unique<BridgeMaker>(*this),
				m_cfg.max_player
			);
		}
		catch (const std::bad_alloc& e)
		{
			// ������� ���
			Logger::Instance().log("Can't allocate memory: %s", e.what());

			// ������������ ������ ����� ������� ��� ������
			throw server_exception(
				std::string("Can't create client pool: " + std::string(e.what()))
			);
		}

		// ������� ���
		Logger::Instance().log("Memory successfully allocated!");
	}

	// ���������� ������ �� ��� ��������
	void Server::destroy_bridge_pool()
	{
		// ������� ���
		Logger::Instance().log("Releasing client memory . . .");

		// ���������� ������� ����� � ���
		m_connected_bridges.for_each(
			[&](Bridge& bridge, bool& stop)
			{
				m_bridge_pool->release(&bridge);
			}
		);

		// ������� ��� ������
		m_bridge_pool.reset();

		// ������� ���
		Logger::Instance().log("Memory released!");
	}

	// ��������� ������� ������
	void Server::init_workers()
	{
		// ������� ����� ������� �������
		m_workers_ready = false;
		m_workers_stop = false;

		// ������� ����� ������� �������
		std::size_t count = static_cast<std::size_t>(
			max(2, std::thread::hardware_concurrency())
		);

		count = 1;

		// ������� ���
		Logger::Instance().log("Starting %d worker threads . . .", count);

		// ������� ������ � ������ ��� ������� ������
		m_workers.reserve(count);

		// ��������� ������� ������
		for (std::size_t i = 0; i < count; ++i)
		{
			// ������� ���� �������
			event_base* evbase = event_base_new();

			// ��������, ��� ���� ������� ���� �������
			if (evbase != nullptr)
			{
				// ��������� ������� �����
				std::thread th(
					std::bind(
						std::mem_fn(&pkodev::Server::work),
						this
					)
				);
				
				// ������� ID ������
				std::thread::id thread_id = th.get_id();

				// ������� �������� � �������� ��� � ������ �������
				m_workers.push_back({evbase, std::move(th)});
				
				// ������� ���
				Logger::Instance().log("Worker thread %d/%d (ID: %04X) successfully started!", (i + 1), count, thread_id);
			}
			else
			{
				// �� ������� ������� ���� �������
				m_workers_stop = true;

				// ������������� ������, ������� ������ �����������
				for (auto& w : m_workers)
				{
					// ������������ ����� �������� � ���������
					if (w.th.joinable() == true)
					{
						w.th.join();
					}

					// ������� ���� �������
					event_base_free(w.evbase);
				}

				// ������� ������ �������
				m_workers.clear();

				// ������� ���
				Logger::Instance().log("Failed to create worker thread %d/%d!", (i + 1), count);

				// ������
				throw server_exception("Failed to create worker threads!");;
			}
		}

		// ������� ��������
		m_workers_ready = true;

		// ������� ���
		Logger::Instance().log("All worker threads are successfully started!");
	}

	// ���������� ������� ������
	void Server::destroy_workers()
	{
		// ������� ���
		Logger::Instance().log("Stopping all worker threads . . .");

		// ������������� ��� ������� ������
		for (auto& w : m_workers)
		{
			// ������������� ���� ������� ��������
			event_base_loopbreak(w.evbase);

			// ������� ID �������� ������ 
			std::thread::id thread_id = w.th.get_id();

			// ������������ ����� � ���������
			if (w.th.joinable() == true)
			{
				w.th.join();
			}	
			
			// ������� ���� �������
			event_base_free(w.evbase);

			// ������� ���
			Logger::Instance().log("Worker thread (ID: %04X) successfully stopped!", thread_id);
		}

		// ������� ������ �������
		m_workers.clear();

		// ������� ���� ���������� �������
		m_workers_ready = false;

		// ������� ���
		Logger::Instance().log("All worker threads are successfully stopped!");
	}

	// ������ ������������� �������� ����������
	void Server::init_listener()
	{
		// ������� ���
		Logger::Instance().log("The process of listening for incoming connections on address (%s:%d) starts . . .", m_cfg.game_host.c_str(), m_cfg.game_port);

		// ��������� ���������� ������
		sockaddr_in local;
		std::memset(reinterpret_cast<void*>(&local), 0x00, sizeof(local));

		// ��������� ��������� ���������� ������
		local.sin_family = AF_INET;
		local.sin_port = htons(m_cfg.game_port);
		int ret = inet_pton(AF_INET, m_cfg.game_host.c_str(), &local.sin_addr);

		// �������� ��������� ������ ������� inet_pton()
		if (ret == SOCKET_ERROR)
		{
			// ������� ���
			Logger::Instance().log("inet_pton() failed! Error code %d: ", WSAGetLastError());

			// ����������� ����������
			throw server_exception("Failed to convert server IP address into its numeric binary form (inet_pton() error).");
		}

		// ������� ���������� ��������
		worker& w = get_min_worker();

		// ��������� ������� ������ �������� ����������
		m_listener = evconnlistener_new_bind(
			w.evbase,
			[](evconnlistener* listener, evutil_socket_t fd,
				sockaddr* address, int length, void* ctx) noexcept
			{
				try
				{
					// �������� ������� ������ �������� ����������
					bool ret = reinterpret_cast<Server*>(ctx)->handle_accept(fd, address);

					// �������� ���������
					if (ret == false)
					{
						// ��������� ����������
						evutil_closesocket(fd);
					}
				}
				catch (...)
				{
					// ������, ��������� ����������
					evutil_closesocket(fd);
				}
			},
			this,
			LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
			-1,
			reinterpret_cast<sockaddr*>(&local),
			sizeof(local)
		);

		

		// ��������, ��� ������� ������������� �������
		if (m_listener == nullptr)
		{
			// ������� ���
			Logger::Instance().log("evconnlistener_new_bind() failed! Is the port already in use?");

			// ������
			throw server_exception(
				std::string("Failed to start listening on address (" + m_cfg.game_host
					+ ":" + std::to_string(m_cfg.game_port) + ")! Is the port already in use?")
			);
		}

		// �������� ����� ������� � ��������
		++w.event_count;

		// ������� ���
		Logger::Instance().log("Ready to accept incoming connections!");
	}

	// ���������� ������������� �������� ����������
	void Server::destroy_listener()
	{
		// ������� ���
		Logger::Instance().log("Stop receiving incoming connections . . .");

		// ������� ������ ��� ������������� �������� ����������
		evconnlistener_free(m_listener); 
		m_listener = nullptr;

		// ������� ���
		Logger::Instance().log("Receiving incoming connections stopped!");
	}

	// ��������� ������
	void Server::run()
	{
		// ������� ���
		Logger::Instance().log("Server starts . . .");

		// ��������, ��� ������ �������
		if (m_running == true)
		{
			// ������� ���
			Logger::Instance().log("Error, the server is already running!");

			// ������ ��� �������
			throw server_exception("Server is already running!");
		}

		// ��������� ������
		try
		{
			// ���� �� ������ �������������
			for (auto it = m_inits.begin(); it != m_inits.end(); ++it)
			{
				// ��������� �������������
				it->init();

				// ��������� ����
				it->initialized = true;
			}
		}
		catch (const std::exception& e)
		{
			// ���������� ���������
			for (auto it = m_inits.rbegin(); it != m_inits.rend(); ++it)
			{
				// ��������, ��� ���� ������������� ������
				if (it->initialized == true)
				{
					// ����������� �������
					it->destroy();

					// ���������� ����
					it->initialized = false;
				}
			}

			// ������������ ���������� �����
			throw server_exception(e.what());
		}

		// ������ �������
		m_running = true;

		// ������� ���
		Logger::Instance().log("Server started!");
	}

	// ���������� ������
	void Server::stop()
	{
		// ������� ���
		Logger::Instance().log("Stopping server . . .");

		// ��������, ��� ������ �������
		if (m_running == false)
		{
			// ������� ���
			Logger::Instance().log("Error, the server is already stopped!");

			// ������ ��� ����������
			return;
		}

		// ����������� �������
		for (auto it = m_inits.rbegin(); it != m_inits.rend(); ++it)
		{
			// ��������, ��� ���� ������������� ������
			if (it->initialized == true)
			{
				// ����������� �������
				it->destroy();

				// ���������� ����
				it->initialized = false;
			}
		}

		// ������ ����������
		m_running = false;

		// ������� ���
		Logger::Instance().log("The server stopped!");
	}
	

	// ������� �������� ����������
	bool Server::handle_accept(evutil_socket_t fd, sockaddr* address)
	{
		// ������������ �������� ����������
		try
		{
			// ��������, ��� �� ������� ���� �����
			if (m_bridge_pool->is_empty() == true)
			{
				// �� ������� �� �������� ��������� ������
				return false;
			}

			// ������� IP-����� � ����
			std::string ip_address = utils::network::get_ip_address(address);
			unsigned short int port = utils::network::get_port(address);

			// �������� IP-����� � ���� �������
			if ( (ip_address.empty() == true) || (port == 0) )
			{
				// ������������ ����� �������
				return false;
			}

			// ��������, ��� �� �������� ����� ����������� � ������ IP-������
			if ( (m_cfg.max_clients_per_ip != 0) &&
					(m_ip_book.get_ip_count(ip_address) >= m_cfg.max_clients_per_ip) )
			{
				// �������� ����� ����������� � ������ IP-������
				return false;
			}

			// �������� ����� ����������� � ������ IP-������
			if ( (m_cfg.connection_interval != 0) &&
					(m_ip_book.is_time_expired(ip_address, m_cfg.connection_interval) == false) )
			{
				// ������� ������ ����������� � ������ IP-������
				return false;
			}

			// ���� ��������, � �������� ����� ���������� �������
			worker& w = get_min_worker();

			// ������ ��� ��������������� ����������� ����� � ��� ��� ������
			auto releaser = [&](Bridge* bridge) noexcept
			{
				// ����������� � ����������� ��� ����������
				try
				{
					// ���������� ���� � ���
					m_bridge_pool->release(bridge);
				}
				catch (...)
				{
					// ������� ����������
					Logger::Instance().log("Caught an exception in bridge releaser on client accept!");
				}
			};
			
			// ����� ���� �� ����
			std::unique_ptr<Bridge, decltype(releaser)> bridge_ptr(
				dynamic_cast<Bridge*>(m_bridge_pool->acquire()), 
				releaser
			);

			// ��������� ������� ���� � Game.exe � ������������� ����� GateServer.exe
			bridge_ptr->build(
				w,
				fd,
				{ ip_address, port }, 
				{ m_cfg.gate_host, m_cfg.gate_port }
			);

			// ��������� ����������� � GateServer.exe
			bridge_ptr->connect();
			
			// ���� ������, ��������� ��� � ������ ������
			add_bridge(bridge_ptr.release());

			// ����������� ������� �����
			++w.event_count;

			// ���������� ������� �������
			return true;
		}
		catch (const bridge_exception& e)
		{
			// ������� ���
			Logger::Instance().log("Exception 'bridge_exception' is occurred on client accept: %s", e.what());
		}
		catch (const object_pool_exception& e)
		{
			// ������� ���
			Logger::Instance().log("Exception 'object_pool_exception' is occurred on client accept: %s", e.what());
		}
		catch (const std::exception& e)
		{
			// ������� ���
			Logger::Instance().log("Exception 'std::exception' is occurred on client accept: %s", e.what());
		}
		catch (...)
		{
			// ������� ���
			Logger::Instance().log("Exception '...' is occurred on client accept");
		}

		// ��������� ������
		return false;
	}

	// ����� �������� � ���������� ������ �������
	Server::worker& Server::get_min_worker()
	{
		// ��������� �� ������ ������� � ������ �������
		auto min_it = m_workers.begin();

		// ���� �������� � ���������� ������ �������
		for (auto it = (min_it + 1); it != m_workers.end(); ++it)
		{
			// ���������� ����� �������
			if (it->event_count.load() < min_it->event_count.load())
			{
				min_it = it;
			}
		}

		return *min_it;
	}

	// ���������, ������� ��������� �������
	void Server::work()
	{
		// ����, ���� ���������� ��� ������� ������
		while (m_workers_ready == false)
		{
			// ��������, ��� ���������� ���������� ��� ������� ������
			if (m_workers_stop == true)
			{
				// ������� �� ���������
				return;
			}

			// ������� 10 ��
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		// ������� ID �������� ������
		std::thread::id this_id = std::this_thread::get_id();

		// ���� ��������, ������� ����������� ������� ������
		worker* w = nullptr;
		for (auto it = m_workers.begin(); it != m_workers.end(); ++it)
		{
			// ���������� ID ������� . . .
			if ( it->th.get_id() == this_id )
			{
				// ������� ������
				w = &(*it);
				break;
			}
		}

		// ��������� ���� �������
		if ( w != nullptr )
		{
			while (event_base_got_break(w->evbase) == false)
			{
				event_base_loop(w->evbase, EVLOOP_NONBLOCK);
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	}

	// �������� ���� � ������
	void Server::add_bridge(Bridge* bridge)
	{
		// ��������� ���� � ������
		bool ret = m_connected_bridges.add(bridge);

		// �������� ���������
		if (ret == true)
		{
			// ��������� IP-����� ������� � ������ �������
			m_ip_book.register_ip(bridge->game_address().ip);
		}
		else
		{
			// �� ������� �������� ���� � ������?
			Logger::Instance().log("Server::add_bridge(): Failed to add a bridge to the list!");
		}
	}

	// ������� ���� �� ������
	void Server::remove_bridge(Bridge* bridge)
	{
		// ������� ���� �� ������
		bool ret = m_connected_bridges.remove(bridge);

		// �������� ���������
		if (ret == true)
		{
			// ������� IP-����� �� ������
			m_ip_book.unregister_ip(
				bridge->game_address().ip,
				m_cfg.connection_interval
			);

			// ���������� ���� � ���
			m_bridge_pool->release(bridge);
		}
		else
		{
			// ���� �� ������ � ������?
			Logger::Instance().log("Server::remove_bridge(): Bridge is not found in the list!");
		}
	}
}