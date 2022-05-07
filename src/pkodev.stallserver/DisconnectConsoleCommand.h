#pragma once
#include "ConsoleCommand.h"

namespace pkodev
{
	// Disconnect clients from the server console command
	class DisconnectConsoleCommand final : public IConsoleCommand
	{
		public:

			// Constructor
			DisconnectConsoleCommand();

			// Destructor
			~DisconnectConsoleCommand() override;

			// Get command name
			std::string name() const override;

			// Get command description
			std::string description() const override;

			// Execute the command
			bool execute(const std::vector<std::string>& params, Server& server) override;

	};
}