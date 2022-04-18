#include "SetStallSuccessPacketHandler.h"
#include "SystemNoticePacket.h"
#include "RingBuffer.h"
#include "Bridge.h"
#include "Utils.h"

namespace pkodev
{
	// Constructor
	SetStallSuccessPacketHandler::SetStallSuccessPacketHandler()
	{

	}

	// Destructor
	SetStallSuccessPacketHandler::~SetStallSuccessPacketHandler()
	{

	}

	// Packet ID
	unsigned short int SetStallSuccessPacketHandler::id() const
	{
		return 858;
	}

	// Packet name
	std::string SetStallSuccessPacketHandler::name() const
	{
		return std::string("Set Stall opened");
	}

	// Transmission direction
	packet_direction_t SetStallSuccessPacketHandler::direction() const
	{
		return packet_direction_t::sc;
	}

	// Read packet
	void SetStallSuccessPacketHandler::read(std::size_t packet_size, LinearBuffer& buffer)
	{

	}

	// Handle packet
	bool SetStallSuccessPacketHandler::handle(Bridge& bridge)
	{
		// Reference on server settings
		const settings_t& settings = bridge.server().settings();

		// Reference on player's game data
		player_data& player = bridge.player();


		// Look for the map on which player is currenlty located in list of maps where the offline stall system works
		auto it = std::find( settings.maps.begin(), settings.maps.end(), utils::string::lower_case(player.map) );

		// Check that player is on map where the offline stall system is disabled
		if (it == settings.maps.end())
		{
			// The player is on map where offline stall system is disabled
			player.offline_stall = false;

			// Send a message to the player's system chat channel
			bridge.send_packet_game(SystemNoticePacket("Offline stall not installed: The offline stall system is disabled on this map."));

			// Pass the packet further
			return true;
		}

		// Check that limitation on number of offline stalls per IP address is enabled
		if (settings.max_stalls_per_ip != 0)
		{
			// Maximum number of offline stalls per IP address
			const std::size_t max_stalls = static_cast<const std::size_t>(settings.max_stalls_per_ip);

			// Counter of offline stalls with the same IP address
			std::size_t counter = 0;

			// Look for offline stalls with the same IP
			bridge.server().offline_bridges().for_each(
				[&bridge, &counter, &max_stalls](Bridge& other)
				{
					// Compare IP addresses
					if (bridge.game_address().ip == other.game_address().ip)
					{
						// Check stalls counter
						if (++counter == max_stalls)
						{
							// Stop the loop
							return true;
						}
					}

					return false;
				}
			);

			// Check stalls counter
			if (counter == max_stalls)
			{
				// The limit of offline stalls on one IP-address has been exceeded
				player.offline_stall = false;

				// Send a message to the player's system chat channel
				bridge.send_packet_game(SystemNoticePacket("Offline stall not installed: The maximum number of offline stalls is set from your IP address."));

				// Pass the packet further
				return true;
			}
		}

		// Offline stall is installed
		player.offline_stall = true;
		
		// Send a message to the player's system chat channel
		bridge.send_packet_game(SystemNoticePacket("Offline stall has been installed: Your character will remain trading after disconnecting from the server."));

		// Pass the packet further
		return true;
	}

	// Validate packet
	bool SetStallSuccessPacketHandler::validate() const
	{
		return true;
	}
}