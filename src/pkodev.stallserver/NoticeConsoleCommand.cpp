#include "NoticeConsoleCommand.h"
#include "Server.h"
#include "Bridge.h"
#include "Utils.h"
#include "SystemNoticePacket.h"
#include "GmNoticePacket.h"

#include <iostream>

namespace pkodev
{
	// Constructor
	NoticeConsoleCommand::NoticeConsoleCommand(const std::string& alias, channel type) :
		m_channel(type),
		m_name(alias)
	{

	}

	// Destructor
	NoticeConsoleCommand::~NoticeConsoleCommand()
	{

	}

	// Get command name
	std::string NoticeConsoleCommand::name() const
	{
		return m_name;
	}

	// Get command description
	std::string NoticeConsoleCommand::description() const
	{
		// The command description
		std::string desc("");

		// Get description
		switch (m_channel)
		{
			case channel::system: desc = "Send a message to the system chat channel to all connected clients."; break;
			case channel::gm: desc = "Send a message to the GM chat channel to all connected clients."; break;
		}

		return desc;
	}

	// Execute the command
	bool NoticeConsoleCommand::execute(const std::vector<std::string>& params, Server& server)
	{
		// Check that we have at least one parameter
		if (params.empty() == true)
		{
			// Wrong syntax
			std::cout << "Wrong syntax: /" << name() << " [message]" << std::endl;
			return false;
		}

		// Build a message
		const std::string message = pkodev::utils::string::join(params);

		// Check message length
		if (message.length() > 511)
		{
			// Too long message
			std::cout << "Wrong command: Too long message" << std::endl;
			return false;
		}

		// A pointer for the notice packet
		std::unique_ptr<IPacket> packet_ptr;

		// Make notice packet
		switch (m_channel)
		{
			case channel::system: packet_ptr = std::make_unique<SystemNoticePacket>(message); break;
			case channel::gm: packet_ptr = std::make_unique<GmNoticePacket>(message); break;
		}

		// Send a message to all connected clients
		server.bridges().for_each(
			[&packet_ptr](Bridge& client) -> bool
			{
				// Check that client is connected
				if (client.game_connected() == true && client.player().cha_id != 0)
				{
					// Send the packet
					client.send_packet_game(*packet_ptr.get());
				}

				return false;
			}
		);

		// Delete the notice packet
		packet_ptr.reset();

		return true;
	}
}