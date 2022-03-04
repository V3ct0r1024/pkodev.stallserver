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
	// Счетчик экземпляров класса
	std::size_t Server::instance_counter = 0;

	// Флаг вывода списка обработчиков
	bool Server::worker::handlers_log = false;


	// Конструктор инициализатора
	Server::initializer::initializer(std::function<void()> init_,
		std::function<void()> destroy_) :
		init(std::move(init_)),
		destroy(std::move(destroy_)),
		initialized(false)
	{

	}

	// Перемещающий конструктор инициализатора
	Server::initializer::initializer(initializer&& other) noexcept :
		init(std::move(other.init)),
		destroy(std::move(other.destroy)),
		initialized(other.initialized)
	{

	}

	// Копирующий конструктор инициализатора
	Server::initializer::initializer(const initializer& other) :
		init(other.init),
		destroy(other.destroy),
		initialized(other.initialized)
	{

	}



	// Конструктор рабочего
	Server::worker::worker(event_base* base, std::thread&& th) :
		evbase(base), 
		th(std::move(th)), 
		event_count(0),
		out_lock(std::make_shared<std::recursive_mutex>()),
		data_lock(std::make_shared<std::recursive_mutex>())
	{
		// Создаем обработчики пакетов
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

		// Выводим список обработчиков
		if (handlers_log == false)
		{
			// Поднимаем флаг
			handlers_log = true;

			// Напечатаем список обработчиков
			{
				// Обработчик
				struct handler_info
				{
					// Поля
					unsigned short int id;    // ID обработчика
					std::string name;         // Название обработчика

					// Конструктор
					handler_info(unsigned short int id_, const std::string& name_) :
						id(id_),
						name(name_)
					{

					}
				};

				// Список интервалов пакетов
				std::vector< std::pair<unsigned short int, unsigned short int> > handler_intervals;

				// Заполняем список интвералов
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

				// Списки обработчиков
				std::vector<handler_info> client_handlers;   // От Game.exe
				std::vector<handler_info> server_handlers;   // От GateServer.exe

				// Собираем список обработчиков
				for (const auto& interval : handler_intervals)
				{
					// Проходим по интвералу
					for (unsigned short int i = interval.first; i < interval.second; ++i)
					{
						// Проверим, что обработчик с данным ID зарегистрирован
						if (handlers->check_handler(i) == true)
						{
							// Получим указатель на обработчик
							const handler_ptr_t& handler = handlers->get_handler(i);

							// Проверим направление перечади
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

				// Условие сортировки обработчиков по ID
				auto sort_cond = [](const handler_info& info1, const handler_info& info2)
				{
					return (info1.id < info2.id);
				};

				// Отсортируем списки по ID
				std::sort(client_handlers.begin(), client_handlers.end(), sort_cond);
				std::sort(server_handlers.begin(), server_handlers.end(), sort_cond);

				// Выводим обработчики пакетов от Game.exe
				{
					// Напишем лог
					Logger::Instance().log("Registered (%d) Game.exe packet handlers:", client_handlers.size());

					// Проходимся по списку обработчиков
					for (const auto& info : client_handlers)
					{
						// Напишем лог
						Logger::Instance().log("* Packet ID %d (%04X): %s.", info.id, info.id, info.name.c_str());
					}
				}

				// Выводим обработчики пакетов от GateServer.exe
				{
					// Напишем лог
					Logger::Instance().log("Registered (%d) GateServer.exe packet handlers:", server_handlers.size());

					// Проходимся по списку обработчиков
					for (const auto& info : server_handlers)
					{
						// Напишем лог
						Logger::Instance().log("* Packet ID %d (%04X): %s.", info.id, info.id, info.name.c_str());
					}
				}
			}
		}
	}

	// Перемещающий конструктор рабочего
	Server::worker::worker(worker&& w) noexcept:
		evbase(w.evbase), 
		th(std::move(w.th)),
		event_count(w.event_count.load()),
		handlers(std::move(w.handlers)),
		out_lock(std::move(w.out_lock)),
		data_lock(std::move(w.data_lock))
	{

	}
	
	
	// Заглушка для функции инициализатора
	static void _nop() {}

	// Конструктор сервера
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

		// Заполнняем список инициализации ресурсов
		{
			// Конечное освобождение ресурсов
			m_inits.push_back(
				{
					_nop,
					std::bind(std::mem_fn(&Server::final_cleanup), this)
				}
			);

			// Загрузка библиотеки WinSock2
			m_inits.push_back(
				{
					std::bind(std::mem_fn(&Server::init_winsock), this),
					std::bind(std::mem_fn(&Server::destroy_winsock), this)
				}
			);

			// Пул сетевых мостов
			m_inits.push_back(
				{
					std::bind(std::mem_fn(&Server::init_bridge_pool), this),
					std::bind(std::mem_fn(&Server::destroy_bridge_pool), this)
				}
			);

			// Рабочие потоки
			m_inits.push_back(
				{
					std::bind(std::mem_fn(&Server::init_workers), this),
					std::bind(std::mem_fn(&Server::destroy_workers), this)
				}
			);

			// Прослушивание входящих соединений
			m_inits.push_back(
				{
					std::bind(std::mem_fn(&Server::init_listener), this),
					std::bind(std::mem_fn(&Server::destroy_listener), this)
				}
			);

			// Первичное освобождение ресурсов
			m_inits.push_back(
				{
					_nop,
					std::bind(std::mem_fn(&Server::initial_cleanup), this)
				}
			);
		}

		// Увеличиваем счетчик экземпляров
		Server::instance_counter++;
	}

	// Деструктор сервера
	Server::~Server()
	{
		// Останавливаем сервер
		stop();

		// Уменьшаем счетчик экземпляров
		Server::instance_counter--;
	}

	// Первичное освобождение ресурсов
	void Server::initial_cleanup()
	{
		// Напишем лог
		Logger::Instance().log("Initial cleanup . . .");


		// Напишем лог
		Logger::Instance().log("Initial cleanup done!");
	}

	// Конечное освобождение ресурсов
	void Server::final_cleanup()
	{
		// Напишем лог
		Logger::Instance().log("Final cleanup . . .");

		// Удаляем список IP-адресов
		m_ip_book.clear();

		// Очищаем список мостов
		m_connected_bridges.clear();
		m_offline_stall_bridges.clear();

		// Напишем лог
		Logger::Instance().log("Final cleanup done!");
	}

	// Загрузить библиотеку WinSock
	void Server::init_winsock()
	{
		// Проверим первый экземпляр сервера
		if (Server::instance_counter == 1)
		{
			// Напишем лог
			Logger::Instance().log("Loading Winsock . . .");

			// Структура с данными о реализации сокетов
			WSAData wsaData;
			std::memset(reinterpret_cast<void*>(&wsaData), 0x00, sizeof(wsaData));

			// Загружаем библиотеку WinSock
			int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);

			// Проверим результат
			if (ret != 0)
			{
				// Выбрасываем исключение
				throw server_exception(
					std::string("WSAStartup() failed with error code " + std::to_string(WSAGetLastError()) + "!")
				);
			}

			// Напишем лог
			Logger::Instance().log("Winsock loaded!");
		}
	}

	// Выгрузить библиотеку WinSock
	void Server::destroy_winsock()
	{
		// Проверим первый экземпляр сервера
		if (Server::instance_counter == 1)
		{
			// Напишем лог
			Logger::Instance().log("Clearing Winsock . . .");

			// Выгружаем библиотеку WinSock
			int ret = WSACleanup();

			// Проверим код ошибки
			if (ret != 0)
			{
				// Напишем лог
				Logger::Instance().log("WSACleanup() failed with error code %d!", WSAGetLastError());
			}
			else
			{
				// Напишем лог
				Logger::Instance().log("Winsock cleared!");
			}
		}
	}

	// Выделить память под клиенты
	void Server::init_bridge_pool()
	{
		// Напишем лог
		Logger::Instance().log("Allocating memory for %d clients . . .", m_cfg.max_player);

		// Создаем пул сетевых мостов
		try
		{
			// Выделяем память под max_player клиетов
			m_bridge_pool = std::make_unique<CObjectPool>(
				std::make_unique<BridgeMaker>(*this),
				m_cfg.max_player
			);
		}
		catch (const std::bad_alloc& e)
		{
			// Напишем лог
			Logger::Instance().log("Can't allocate memory: %s", e.what());

			// Недостаточно памяти чтобы создать пул мостов
			throw server_exception(
				std::string("Can't create client pool: " + std::string(e.what()))
			);
		}

		// Напишем лог
		Logger::Instance().log("Memory successfully allocated!");
	}

	// Освободить память из под клиентов
	void Server::destroy_bridge_pool()
	{
		// Напишем лог
		Logger::Instance().log("Releasing client memory . . .");

		// Возвращаем сетевые мосты в пул
		m_connected_bridges.for_each(
			[&](Bridge& bridge, bool& stop)
			{
				m_bridge_pool->release(&bridge);
			}
		);

		// Удаляем пул мостов
		m_bridge_pool.reset();

		// Напишем лог
		Logger::Instance().log("Memory released!");
	}

	// Запустить рабочие потоки
	void Server::init_workers()
	{
		// Сбросим флаги запуска рабочих
		m_workers_ready = false;
		m_workers_stop = false;

		// Получим число рабочих потоков
		std::size_t count = static_cast<std::size_t>(
			max(2, std::thread::hardware_concurrency())
		);

		count = 1;

		// Напишем лог
		Logger::Instance().log("Starting %d worker threads . . .", count);

		// Выделим память в списке под рабочие потоки
		m_workers.reserve(count);

		// Запускаем рабочие потоки
		for (std::size_t i = 0; i < count; ++i)
		{
			// Создаем ядро событий
			event_base* evbase = event_base_new();

			// Проверим, что ядро событий было создано
			if (evbase != nullptr)
			{
				// Запускаем рабочий поток
				std::thread th(
					std::bind(
						std::mem_fn(&pkodev::Server::work),
						this
					)
				);
				
				// Получим ID потока
				std::thread::id thread_id = th.get_id();

				// Создаем рабочего и помещаем его в список рабочих
				m_workers.push_back({evbase, std::move(th)});
				
				// Напишем лог
				Logger::Instance().log("Worker thread %d/%d (ID: %04X) successfully started!", (i + 1), count, thread_id);
			}
			else
			{
				// Не удалось создать ядро событий
				m_workers_stop = true;

				// Останавливаем потоки, которые успели запуститься
				for (auto& w : m_workers)
				{
					// Присоединяем поток рабочего к основному
					if (w.th.joinable() == true)
					{
						w.th.join();
					}

					// Удаляем ядро событий
					event_base_free(w.evbase);
				}

				// Очищаем список рабочих
				m_workers.clear();

				// Напишем лог
				Logger::Instance().log("Failed to create worker thread %d/%d!", (i + 1), count);

				// Ошибка
				throw server_exception("Failed to create worker threads!");;
			}
		}

		// Рабочие запущены
		m_workers_ready = true;

		// Напишем лог
		Logger::Instance().log("All worker threads are successfully started!");
	}

	// Остановить рабочие потоки
	void Server::destroy_workers()
	{
		// Напишем лог
		Logger::Instance().log("Stopping all worker threads . . .");

		// Останавливаем все рабочие потоки
		for (auto& w : m_workers)
		{
			// Останавливаем цикл событий рабочего
			event_base_loopbreak(w.evbase);

			// Получим ID рабочего потока 
			std::thread::id thread_id = w.th.get_id();

			// Присоединяем поток к основному
			if (w.th.joinable() == true)
			{
				w.th.join();
			}	
			
			// Удаляем ядро событий
			event_base_free(w.evbase);

			// Напишем лог
			Logger::Instance().log("Worker thread (ID: %04X) successfully stopped!", thread_id);
		}

		// Очищаем список рабочих
		m_workers.clear();

		// Сбросим флаг готовности рабочих
		m_workers_ready = false;

		// Напишем лог
		Logger::Instance().log("All worker threads are successfully stopped!");
	}

	// Начать прослушивание входящих соединений
	void Server::init_listener()
	{
		// Напишем лог
		Logger::Instance().log("The process of listening for incoming connections on address (%s:%d) starts . . .", m_cfg.game_host.c_str(), m_cfg.game_port);

		// Структура локального адреса
		sockaddr_in local;
		std::memset(reinterpret_cast<void*>(&local), 0x00, sizeof(local));

		// Заполняем структуру локального адреса
		local.sin_family = AF_INET;
		local.sin_port = htons(m_cfg.game_port);
		int ret = inet_pton(AF_INET, m_cfg.game_host.c_str(), &local.sin_addr);

		// Проверим результат работы функции inet_pton()
		if (ret == SOCKET_ERROR)
		{
			// Напишем лог
			Logger::Instance().log("inet_pton() failed! Error code %d: ", WSAGetLastError());

			// Выбрасываем исключение
			throw server_exception("Failed to convert server IP address into its numeric binary form (inet_pton() error).");
		}

		// Получим свободного рабочего
		worker& w = get_min_worker();

		// Запускаем процесс приема входящих соединений
		m_listener = evconnlistener_new_bind(
			w.evbase,
			[](evconnlistener* listener, evutil_socket_t fd,
				sockaddr* address, int length, void* ctx) noexcept
			{
				try
				{
					// Вызываем событие приема входящих соединений
					bool ret = reinterpret_cast<Server*>(ctx)->handle_accept(fd, address);

					// Проверим результат
					if (ret == false)
					{
						// Закрываем соединение
						evutil_closesocket(fd);
					}
				}
				catch (...)
				{
					// Ошибка, закрываем соединение
					evutil_closesocket(fd);
				}
			},
			this,
			LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
			-1,
			reinterpret_cast<sockaddr*>(&local),
			sizeof(local)
		);

		

		// Проверим, что процесс прослушивания запущен
		if (m_listener == nullptr)
		{
			// Напишем лог
			Logger::Instance().log("evconnlistener_new_bind() failed! Is the port already in use?");

			// Ошибка
			throw server_exception(
				std::string("Failed to start listening on address (" + m_cfg.game_host
					+ ":" + std::to_string(m_cfg.game_port) + ")! Is the port already in use?")
			);
		}

		// Увеличим число событий у рабочего
		++w.event_count;

		// Напишем лог
		Logger::Instance().log("Ready to accept incoming connections!");
	}

	// Остановить прослушивание входящих соединений
	void Server::destroy_listener()
	{
		// Напишем лог
		Logger::Instance().log("Stop receiving incoming connections . . .");

		// Удаляем объект для прослушивания входящих соединений
		evconnlistener_free(m_listener); 
		m_listener = nullptr;

		// Напишем лог
		Logger::Instance().log("Receiving incoming connections stopped!");
	}

	// Запустить сервер
	void Server::run()
	{
		// Напишем лог
		Logger::Instance().log("Server starts . . .");

		// Проверим, что сервер запущен
		if (m_running == true)
		{
			// Напишем лог
			Logger::Instance().log("Error, the server is already running!");

			// Сервер уже запущен
			throw server_exception("Server is already running!");
		}

		// Запускаем сервер
		try
		{
			// Цикл по списку инициализации
			for (auto it = m_inits.begin(); it != m_inits.end(); ++it)
			{
				// Выполняем инициализацию
				it->init();

				// Поднимаем флаг
				it->initialized = true;
			}
		}
		catch (const std::exception& e)
		{
			// Откатываем изменения
			for (auto it = m_inits.rbegin(); it != m_inits.rend(); ++it)
			{
				// Проверим, что флаг инициализации поднят
				if (it->initialized == true)
				{
					// Освобождаем ресурсы
					it->destroy();

					// Сбрасываем флаг
					it->initialized = false;
				}
			}

			// Пробрасываем исключение далее
			throw server_exception(e.what());
		}

		// Сервер запущен
		m_running = true;

		// Напишем лог
		Logger::Instance().log("Server started!");
	}

	// Остановить сервер
	void Server::stop()
	{
		// Напишем лог
		Logger::Instance().log("Stopping server . . .");

		// Проверим, что сервер запущен
		if (m_running == false)
		{
			// Напишем лог
			Logger::Instance().log("Error, the server is already stopped!");

			// Сервер уже остановлен
			return;
		}

		// Освобождаем ресурсы
		for (auto it = m_inits.rbegin(); it != m_inits.rend(); ++it)
		{
			// Проверим, что флаг инициализации поднят
			if (it->initialized == true)
			{
				// Освобождаем ресурсы
				it->destroy();

				// Сбрасываем флаг
				it->initialized = false;
			}
		}

		// Сервер остановлен
		m_running = false;

		// Напишем лог
		Logger::Instance().log("The server stopped!");
	}
	

	// Принять входящее соединение
	bool Server::handle_accept(evutil_socket_t fd, sockaddr* address)
	{
		// Обрабатываем входящее соединение
		try
		{
			// Проверим, что на сервере есть место
			if (m_bridge_pool->is_empty() == true)
			{
				// На сервере не осталось свободных слотов
				return false;
			}

			// Получим IP-адрес и порт
			std::string ip_address = utils::network::get_ip_address(address);
			unsigned short int port = utils::network::get_port(address);

			// Проверим IP-адрес и порт клиента
			if ( (ip_address.empty() == true) || (port == 0) )
			{
				// Некорректный адрес клиента
				return false;
			}

			// Проверим, что не превышен лимит подключений с одного IP-адреса
			if ( (m_cfg.max_clients_per_ip != 0) &&
					(m_ip_book.get_ip_count(ip_address) >= m_cfg.max_clients_per_ip) )
			{
				// Превышен лимит подключений с одного IP-адреса
				return false;
			}

			// Проверим время подключения с одного IP-адреса
			if ( (m_cfg.connection_interval != 0) &&
					(m_ip_book.is_time_expired(ip_address, m_cfg.connection_interval) == false) )
			{
				// Слишком частые подключения с одного IP-адреса
				return false;
			}

			// Ищем рабочего, к которому можно прикрепить клиента
			worker& w = get_min_worker();

			// Лямбда для автоматического возвращения моста в пул при ошибке
			auto releaser = [&](Bridge* bridge) noexcept
			{
				// Отлавливаем и обратаываем все исключения
				try
				{
					// Возвращаем мост в пул
					m_bridge_pool->release(bridge);
				}
				catch (...)
				{
					// Поймали исключение
					Logger::Instance().log("Caught an exception in bridge releaser on client accept!");
				}
			};
			
			// Берем мост из пула
			std::unique_ptr<Bridge, decltype(releaser)> bridge_ptr(
				dynamic_cast<Bridge*>(m_bridge_pool->acquire()), 
				releaser
			);

			// Связываем сетевой мост с Game.exe и устанавливаем адрес GateServer.exe
			bridge_ptr->build(
				w,
				fd,
				{ ip_address, port }, 
				{ m_cfg.gate_host, m_cfg.gate_port }
			);

			// Запускаем подключение к GateServer.exe
			bridge_ptr->connect();
			
			// Мост создан, добавляем его в список мостов
			add_bridge(bridge_ptr.release());

			// Увеличиваем счетчик задач
			++w.event_count;

			// Соединение успешно принято
			return true;
		}
		catch (const bridge_exception& e)
		{
			// Напишем лог
			Logger::Instance().log("Exception 'bridge_exception' is occurred on client accept: %s", e.what());
		}
		catch (const object_pool_exception& e)
		{
			// Напишем лог
			Logger::Instance().log("Exception 'object_pool_exception' is occurred on client accept: %s", e.what());
		}
		catch (const std::exception& e)
		{
			// Напишем лог
			Logger::Instance().log("Exception 'std::exception' is occurred on client accept: %s", e.what());
		}
		catch (...)
		{
			// Напишем лог
			Logger::Instance().log("Exception '...' is occurred on client accept");
		}

		// Произошла ошибка
		return false;
	}

	// Найти рабочего с наименьшим числом событий
	Server::worker& Server::get_min_worker()
	{
		// Указатель на первый элемент в списке рабочих
		auto min_it = m_workers.begin();

		// Ищем рабочего с наименьшим числом событий
		for (auto it = (min_it + 1); it != m_workers.end(); ++it)
		{
			// Сравниваем число событий
			if (it->event_count.load() < min_it->event_count.load())
			{
				min_it = it;
			}
		}

		return *min_it;
	}

	// Процедура, которую выполняет рабочий
	void Server::work()
	{
		// Ждем, пока запустятся все рабочие потоки
		while (m_workers_ready == false)
		{
			// Проверим, что необходимо остановить все рабочие потоки
			if (m_workers_stop == true)
			{
				// Выходим из процедуры
				return;
			}

			// Ожидаем 10 мс
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		// Получим ID текущего потока
		std::thread::id this_id = std::this_thread::get_id();

		// Ищем рабочего, которые принадлежит данному потоку
		worker* w = nullptr;
		for (auto it = m_workers.begin(); it != m_workers.end(); ++it)
		{
			// Сравниваем ID потоков . . .
			if ( it->th.get_id() == this_id )
			{
				// Рабочий найден
				w = &(*it);
				break;
			}
		}

		// Выполняем цикл событий
		if ( w != nullptr )
		{
			while (event_base_got_break(w->evbase) == false)
			{
				event_base_loop(w->evbase, EVLOOP_NONBLOCK);
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	}

	// Добавить мост в список
	void Server::add_bridge(Bridge* bridge)
	{
		// Добавляем мост в список
		bool ret = m_connected_bridges.add(bridge);

		// Проверим результат
		if (ret == true)
		{
			// Добавляем IP-адрес клиента в список адресов
			m_ip_book.register_ip(bridge->game_address().ip);
		}
		else
		{
			// Не удалось добавить мост в список?
			Logger::Instance().log("Server::add_bridge(): Failed to add a bridge to the list!");
		}
	}

	// Удалить мост из списка
	void Server::remove_bridge(Bridge* bridge)
	{
		// Удаляем мост из списка
		bool ret = m_connected_bridges.remove(bridge);

		// Проверим результат
		if (ret == true)
		{
			// Удаляем IP-адрес из списка
			m_ip_book.unregister_ip(
				bridge->game_address().ip,
				m_cfg.connection_interval
			);

			// Возвращаем мост в пул
			m_bridge_pool->release(bridge);
		}
		else
		{
			// Мост не найден в списке?
			Logger::Instance().log("Server::remove_bridge(): Bridge is not found in the list!");
		}
	}
}