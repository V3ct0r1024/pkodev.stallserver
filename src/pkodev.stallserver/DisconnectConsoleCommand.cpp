#include "DisconnectConsoleCommand.h"
#include "Server.h"
#include "Bridge.h"
#include "Utils.h"

#include <iostream>

namespace pkodev
{
	// Constructor
	DisconnectConsoleCommand::DisconnectConsoleCommand()
	{

	}

	// Destructor
	DisconnectConsoleCommand::~DisconnectConsoleCommand()
	{

	}

	// Get command name
	std::string DisconnectConsoleCommand::name() const
	{
		return std::string("disconnect");
	}

	// Get command description
	std::string DisconnectConsoleCommand::description() const
	{
		return std::string("Disconnect clients: [all] - all clients; [offline] - offline stalls only.");
	}

	// Execute the command
	bool DisconnectConsoleCommand::execute(const std::vector<std::string>& params, Server& server)
	{
		// Check that we have one parameter
		if (params.size() != 1)
		{
			// Wrong syntax
			std::cout << "Wrong syntax: /" << name() << " [all|offline]" << std::endl;
			return false;
		}

		// Make parameter
		const std::string type = utils::string::lower_case(params[0]);

		// Check that parameter is correct
		if (type != "all" && type != "offline")
		{
			// Wrong parameter
			std::cout << "Wrong parameter: /" << name() << " [all|offline]" << std::endl;
			return false;
		}

		// Get the reference to the bridges list
		BridgeList& clients = ( (type == "offline") ? server.offline_bridges() : server.bridges() );
		
		// The number of disconnected clients
		std::size_t counter = clients.count();

		// Disconnect all clients
		clients.for_each(
			[](Bridge& client) -> bool
			{
				client.disconnect();
				return false;
			}
		);

		// Print a message
		std::cout << "(" << counter << ") clients disconnected!" << std::endl;

		return true;
	}
}