#pragma once
#include "ConsoleCommand.h"

namespace pkodev
{
	// Stop server
	class StopServerConsoleCommand final: public IConsoleCommand
	{
		public:

			// Constructor
			StopServerConsoleCommand();

			// Destructor
			~StopServerConsoleCommand();

			// Get command name
			std::string name() const override;

			// Get command description
			std::string description() const override;

			// Execute the command
			bool execute(const std::vector<std::string>& params, Server& server) override;

	};
}


