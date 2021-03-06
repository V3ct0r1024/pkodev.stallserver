#include "StopServerConsoleCommand.h"
#include "Server.h"

namespace pkodev
{
	// Constructor
	StopServerConsoleCommand::StopServerConsoleCommand() :
		m_name("stop")
	{

	}

	// Constructor
	StopServerConsoleCommand::StopServerConsoleCommand(const std::string& name) :
		m_name(name)
	{

	}

	// Destructor
	StopServerConsoleCommand::~StopServerConsoleCommand()
	{

	}

	// Get command name
	std::string StopServerConsoleCommand::name() const
	{
		return m_name;
	}

	// Get command description
	std::string StopServerConsoleCommand::description() const
	{
		return std::string("Stop the server.");
	}

	// Execute the command
	bool StopServerConsoleCommand::execute(const std::vector<std::string>& params, Server& server)
	{
		// Stop the server
		if (server.is_running() == true)
		{
			server.stop();
			return true;
		}

		return false;
	}
}