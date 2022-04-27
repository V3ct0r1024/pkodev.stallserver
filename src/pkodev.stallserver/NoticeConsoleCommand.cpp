#include "NoticeConsoleCommand.h"
#include "Server.h"
#include "Bridge.h"
#include "SystemNoticePacket.h"

#include <iostream>

namespace pkodev
{
	// Constructor
	NoticeConsoleCommand::NoticeConsoleCommand()
	{

	}

	// Destructor
	NoticeConsoleCommand::~NoticeConsoleCommand()
	{

	}

	// Get command name
	std::string NoticeConsoleCommand::name() const
	{
		return std::string("notice");
	}

	// Get command description
	std::string NoticeConsoleCommand::description() const
	{
		return std::string("Send a message to the system chat channel to all connected clients.");
	}

	// Execute the command
	bool NoticeConsoleCommand::execute(const std::vector<std::string>& params, Server& server)
	{
		// Check that we have one parameter
		if (params.size() != 1)
		{
			// Wrong syntax
			std::cout << "Wrong syntax: /" << name() << " [message]" << std::endl;
			return false;
		}

		// Check message length
		if (params[0].length() > 512)
		{
			// Too long message
			std::cout << "Wrong command: Too long message" << std::endl;
			return false;
		}

		// Make system notice packet
		const SystemNoticePacket packet(params[0]);

		// Send a message to all connected clients
		server.bridges().for_each(
			[&packet](Bridge& client) -> bool
			{
				// Check that client is connected
				if (client.game_connected() == true && client.player().cha_id != 0)
				{
					// Send the packet
					client.send_packet_game(packet);
				}

				return false;
			}
		);

		return true;
	}
}