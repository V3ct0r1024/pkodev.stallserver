#pragma once
#include "ConsoleCommand.h"

namespace pkodev
{
	// Kick a client by character or account from the server console command
	class KickConsoleCommand final : public IConsoleCommand
	{
		public:

			// Constructor
			KickConsoleCommand();

			// Destructor
			~KickConsoleCommand() override;

			// Get command name
			std::string name() const override;

			// Get command description
			std::string description() const override;

			// Execute the command
			bool execute(const std::vector<std::string>& params, Server& server) override;
	};
}