#include "StatConsoleCommand.h"
#include "Server.h"

#include <iostream>
#include <iomanip>
#include <chrono>

using namespace std::chrono;

namespace pkodev
{

	// Constructor
	StatConsoleCommand::StatConsoleCommand()
	{

	}

	// Destructor
	StatConsoleCommand::~StatConsoleCommand()
	{

	}

	// Get command name
	std::string StatConsoleCommand::name() const
	{
		return std::string("stat");
	}

	// Get command description
	std::string StatConsoleCommand::description() const
	{
		return std::string("Show server statistics.");
	}

	// Execute the command
	bool StatConsoleCommand::execute(const std::vector<std::string>& params, Server& server)
	{
		// Get the number of connected clients
		const unsigned int clients_counter = server.bridges().count();

		// Get the number of offline stalls
		const unsigned int offline_counter = server.offline_bridges().count();

		// Get workers info
		const auto workers = server.make_worker_info();

		// Get server settings
		const settings_t& settings = server.settings();

		// Calculate server uptime
		const auto uptime = duration_cast<seconds>(system_clock::now() - server.startup_time()).count();


		// Print statistics
		std::cout << "Server statistics" << std::endl;
		std::cout << "* Uptime: "            << uptime << " seconds"                            << std::endl;
		std::cout << "* Clients connected: " << clients_counter << " / " << settings.max_player << std::endl;
		std::cout << "* Offline stalls: "    << offline_counter                                 << std::endl;
		std::cout << "* Threads: "           << workers.size()                                  << std::endl;

		// Print table header
		std::cout << '+' << std::setfill('-') << std::setw(7) << '+'         << std::setw(17) << '+'                 << std::setw(17) << '+'                  << std::setw(25)                               << '+' << std::endl;
		std::cout << '|' << std::setfill(' ') << std::setw(6) << "# " << '|' << std::setw(16) << "Thread ID " << '|' << std::setw(16) << "Task count " << '|' << std::setw(24) << "Max operation time (ms)"  << '|' << std::endl;
		std::cout << '+' << std::setfill('-') << std::setw(7) << '+'         << std::setw(17) << '+'                 << std::setw(17) << '+'                  << std::setw(25)                               << '+' << std::endl;
		
		// Print table items
		for (unsigned int i  = 0; i < workers.size(); ++i)
		{
			std::cout << '|' << std::setfill(' ') << std::setw(5)
			  	      << (i + 1)                                           << ' ' << '|' << std::setw(15) << std::hex
				      << workers[i].thread_id                              << ' ' << '|' << std::setw(15) << std::dec
				      << workers[i].tasks                                  << ' ' << '|' << std::setw(23)
				      << static_cast<double>(workers[i].max_time / 1000.0) << ' ' << '|' << std::endl;
		}

		// Print table bottom border
		std::cout << '+' << std::setfill('-') << std::setw(7) << '+'         << std::setw(17) << '+'                 << std::setw(17) << '+'                  << std::setw(25)                               << '+' << std::endl;


		return true;
	}
}