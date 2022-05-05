#pragma once
#include "ConsoleCommand.h"

namespace pkodev
{
	class KickConsoleCommand final : public IConsoleCommand
	{
		public:

			// Constructor
			KickConsoleCommand();

			// Destructor
			~KickConsoleCommand();

			// Get command name
			std::string name() const override;

			// Get command description
			std::string description() const override;

			// Execute the command
			bool execute(const std::vector<std::string>& params, Server& server) override;
	};
}