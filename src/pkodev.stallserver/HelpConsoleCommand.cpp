#include "HelpConsoleCommand.h"
#include "Server.h"
#include <iostream>

namespace pkodev
{
	// Constructor
	HelpConsoleCommand::HelpConsoleCommand()
	{

	}

	// Destructor
	HelpConsoleCommand::~HelpConsoleCommand()
	{

	}

	// Get command name
	std::string HelpConsoleCommand::name() const
	{
		return std::string("help");
	}

	// Get command description
	std::string HelpConsoleCommand::description() const
	{
		return std::string("Show available console commands.");
	}

	// Execute the command
	bool HelpConsoleCommand::execute(const std::vector<std::string>& params, Server& server)
	{
		// Get console commands
		const auto& commands = server.console_commands();

		// Print message
		std::cout << "Available commands: " << std::endl;

		// Print commands
		if (commands.empty() == false)
		{
			std::size_t n = 0;

			for (const auto& c : commands)
			{
				std::cout << ++n << ") '/" << c.second->name() << "' - " << c.second->description() << std::endl;
			}
		}
		else
		{
			std::cout << "No commands!" << std::endl;
		}
		
		return true;
	}
}