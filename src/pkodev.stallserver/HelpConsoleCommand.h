#pragma once
#include "ConsoleCommand.h"

namespace pkodev
{
	// Show available console commands console command
	class HelpConsoleCommand final : public IConsoleCommand
	{
		public:

			// Constructor
			HelpConsoleCommand();

			// Destructor
			~HelpConsoleCommand() override;

			// Get command name
			std::string name() const override;

			// Get command description
			std::string description() const override;

			// Execute the command
			bool execute(const std::vector<std::string>& params, Server& server) override;

	};
}
