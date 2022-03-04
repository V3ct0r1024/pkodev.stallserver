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
	// Определим тип сетевого моста
	class Bridge;
	class BridgeList;

	// Определим тип списка карт, на которых работает система оффлайн ларьков
	typedef std::vector<std::string> map_list_t;

	// Исключение для класса сервера
	class server_exception final : public std::runtime_error
	{
		public:

			// Конструктор с const char *
			server_exception(const char* what) :
				std::runtime_error(what) { }

			// Конструктор с std::string
			server_exception(const std::string& what) :
				std::runtime_error(what) { }
	};

	// Структура с настройками сервера
	struct settings_t final
	{
		// Настройки подключения к StallServer.exe
		std::string game_host;                  // Адрес Game.exe -> StallServer.exe
		unsigned short int game_port;           // Порт  Game.exe -> StallServer.exe
		unsigned short int max_player;          // Максимальное число клиентов, которые одновременно могут подключиться к серверу
		unsigned short int max_clients_per_ip;  // Максимальное число клиентов с одинаковым IP-адресом
		unsigned int connection_interval;       // Интервал времени между подключениями с одного IP-адреса

		// Настройки подключения к GateServer.exe
		std::string gate_host;         // Адрес StallServer.exe -> GateServer.exe
		unsigned short int gate_port;  // Порт  StallServer.exe -> GateServer.exe

		// Список карт, на которых работает система оффлайн ларьков
		map_list_t maps;

		// Максимальное число ларьков с 1 IP-адреса
		unsigned short int max_stalls_per_ip;

		// Максимальное время торговли в оффлайн ларьке в секундах
		unsigned int max_offline_time;

		// Конструктор
		settings_t() :
			game_host(""), game_port(0), max_player(0), 
			max_clients_per_ip(0), connection_interval(0),
			gate_host(""), gate_port(0),
			max_stalls_per_ip(0), max_offline_time(0)
		{
			maps.reserve(8);
		}
	};

	// Класс сервера
	class Server final
	{
		public:

			// Конструктор
			Server(const settings_t& settings_);

			// Конструктор копирования
			Server(const Server&) = delete;

			// Конструктор перемещения
			Server(Server&&) = delete;

			// Деструктор
			~Server();

			// Оператор присваивания копированием
			Server& operator=(const Server&) = delete;

			// Оператор присваивания перемещением
			Server& operator=(Server&&) = delete;

			// Запустить сервер
			void run();

			// Остановить сервер
			void stop();

			// Получить ссылку на настройки
			inline const settings_t& settings() const { return m_cfg; }

			// Запущен ли сервер
			inline bool is_running() const { return m_running; }

			// Получить список мостов
			inline BridgeList& bridges() { return m_connected_bridges; }

			// Получить список мостов в режиме оффлайн ларька
			inline BridgeList& offline_bridges() { return m_offline_stall_bridges; }


		private:

			friend class Bridge;

			// Структура инициализатора
			struct initializer final
			{
				// Инициализация
				std::function<void()> init;

				// Освобождение ресурсов
				std::function<void()> destroy;

				// Флаг инициализации
				bool initialized;

				// Конструктор
				initializer(std::function<void()> init_, std::function<void()> destroy_);

				// Перемещающий конструктор
				initializer(initializer&& other) noexcept;

				// Копирующий конструктор
				initializer(const initializer& other);
			};

			// Структура рабочего
			struct worker final
			{
				// Флаг вывода списка обработчиков
				static bool handlers_log;

				// Ядро событий
				event_base* evbase;

				// Поток
				std::thread th;

				// Число задач
				std::atomic_int event_count;

				// Обработчики пакетов
				std::shared_ptr<PacketHandlerStorage> handlers;

				// A lock for output and disconnection operations
				std::shared_ptr<std::recursive_mutex> out_lock;

				// A lock for player-related data
				std::shared_ptr<std::recursive_mutex> data_lock;


				// Конструктор
				worker(event_base* base, std::thread&& th);

				// Перемещающий конструктор
				worker(worker&& other) noexcept;

				// Копирующий конструктор
				worker(const worker& other) = delete;
			};


			// Первичное и конечное освобождение ресурсов
			void initial_cleanup();
			void final_cleanup();

			// Загрузить библиотеку WinSock
			void init_winsock();
			void destroy_winsock();
			
			// Выделение памяти под клиенты
			void init_bridge_pool();
			void destroy_bridge_pool();

			// Инициализация рабочих потоков
			void init_workers();
			void destroy_workers();

			// Инициализация прослушивания входящих соединений
			void init_listener();
			void destroy_listener();


			// Принять входящее соединение
			bool handle_accept(evutil_socket_t fd, sockaddr* address);

			// Процедура, которую выполняет рабочий
			void work();

			// Найти рабочего с наименьшим числом событий
			worker& get_min_worker();

			// Добавить мост в список
			void add_bridge(Bridge* bridge);

			// Удалить мост из списка
			void remove_bridge(Bridge* bridge);


			// Счетчик экземпляров класса
			static std::size_t instance_counter;


			// Список инициализаторов
			std::vector<initializer> m_inits;


			// Ссылка на настройки
			const settings_t& m_cfg;
			

			// Запущен ли сервер
			bool m_running;

			// Список рабочих потоков
			std::vector<worker> m_workers;

			// Запущены ли все рабочие потоки
			std::atomic<bool> m_workers_ready;

			// Нужно ли остановить рабочие потоки
			std::atomic<bool> m_workers_stop;

			// Объект для прослушивания входящих соединений
			evconnlistener* m_listener;

			// Пул сетевых мостов
			std::unique_ptr<CObjectPool> m_bridge_pool;

			// Список всех подключенных мостов
			BridgeList m_connected_bridges;

			// Список мостов, которые находятся в режиме оффлайн ларька
			BridgeList m_offline_stall_bridges;

			// Список подключенных IP адресов
			IpAddressBook m_ip_book;
	};
}