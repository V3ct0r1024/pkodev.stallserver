#pragma once
#include <string>
#include <vector>

namespace pkodev
{
	class Server;

	// Administartor console command interface
	class IConsoleCommand
	{
		public:

			// Get command name
			virtual std::string name() const = 0;

			// Get command description
			virtual std::string description() const = 0;

			// Execute the command
			virtual bool execute(const std::vector<std::string>& params, Server& server) = 0;

	};

}
