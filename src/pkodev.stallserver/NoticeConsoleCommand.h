#pragma once
#include "ConsoleCommand.h"

namespace pkodev
{
	class NoticeConsoleCommand final : public IConsoleCommand
	{
		public:

			// Constructor
			NoticeConsoleCommand();

			// Destructor
			~NoticeConsoleCommand();

			// Get command name
			std::string name() const override;

			// Get command description
			std::string description() const override;

			// Execute the command
			bool execute(const std::vector<std::string>& params, Server& server) override;

	};
}

