#pragma once
#include "ConsoleCommand.h"

namespace pkodev
{
	// Send a message to all connected clients in system/GM chat channel console command
	class NoticeConsoleCommand final : public IConsoleCommand
	{
		public:

			// Chat channel type
			enum class channel : unsigned int
			{
				system = 0,
				gm
			};

			// Constructor
			NoticeConsoleCommand(const std::string& alias, NoticeConsoleCommand::channel type);

			// Destructor
			~NoticeConsoleCommand() override;

			// Get command name
			std::string name() const override;

			// Get command description
			std::string description() const override;

			// Execute the command
			bool execute(const std::vector<std::string>& params, Server& server) override;

		private:

			// Channel
			channel m_channel;

			// Command name
			std::string m_name;

	};
}

