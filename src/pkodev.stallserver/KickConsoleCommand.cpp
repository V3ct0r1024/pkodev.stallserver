#include "KickConsoleCommand.h"
#include "Server.h"
#include "Bridge.h"
#include "Utils.h"

#include <iostream>


namespace pkodev
{
	// Constructor
	KickConsoleCommand::KickConsoleCommand()
	{

	}

	// Destructor
	KickConsoleCommand::~KickConsoleCommand()
	{

	}

	// Get command name
	std::string KickConsoleCommand::name() const
	{
		return std::string("kick");
	}

	// Get command description
	std::string KickConsoleCommand::description() const
	{
		return std::string("Disconnect a client by character name or account: [character] [character name] - by character name; [account] [account name] - by account.");
	}

	// Execute the command
	bool KickConsoleCommand::execute(const std::vector<std::string>& params, Server& server)
	{
		// Check that we have one parameter
		if (params.size() != 2)
		{
			// Wrong syntax
			std::cout << "Wrong syntax: /" << name() << " [character|account] [name]" << std::endl;
			return false;
		}

		// Make parameter
		const std::string type = utils::string::lower_case(params[0]);

		// Check that parameter is correct
		if (type != "character" && type != "account")
		{
			// Wrong parameter
			std::cout << "Wrong parameter: /" << name() << " [character|account] [name]" << std::endl;
			return false;
		}

		// Get a reference to the connected clients list
		BridgeList& clients = server.bridges();

		// Search for a client
		const auto& opt = (type == "character") ? clients.find_by_character(params[1]) : clients.find_by_account(params[1]);

		// Check that client is found
		if (opt.has_value() == true)
		{
			// Disconnect the client
			opt.value()->disconnect();

			// Print a message
			std::cout << ( (type == "character") ? "Character" : "Account" ) << " '" << params[1] << "' has been disconnected!" << std::endl;
		}
		else
		{
			// Print a message
			std::cout << ((type == "character") ? "Character" : "Account") << " '" << params[1] << "' not found!" << std::endl;

			// Command failed
			return false;
		}

		return true;
	}
}