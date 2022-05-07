#pragma once
#include "ConsoleCommand.h"

namespace pkodev
{
	// Show the server statistics console command
	class StatConsoleCommand final : public IConsoleCommand
	{
		public:

			// Constructor
			StatConsoleCommand();

			// Destructor
			~StatConsoleCommand() override;

			// Get command name
			std::string name() const override;

			// Get command description
			std::string description() const override;

			// Execute the command
			bool execute(const std::vector<std::string>& params, Server& server) override;
	};
}

