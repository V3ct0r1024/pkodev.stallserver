#include "BridgeList.h"
#include "Bridge.h"
#include "Utils.h"

namespace pkodev
{
	// Constructor
	BridgeList::BridgeList()
	{
		// Reserve some memory
		m_bridges.reserve(128);
	}

	// Destructor
	BridgeList::~BridgeList()
	{

	}

	// Add a network bridge to the list
	bool BridgeList::add(Bridge* bridge)
	{
		// Check pointer to the network bridge
		if (bridge == nullptr)
		{
			return false;
		}

		{
			// Lock the list of network bridges
			std::lock_guard<std::mutex> lock(m_mtx);
			
			// Look for the network bridge in the list
			auto it = std::find(m_bridges.begin(), m_bridges.end(), bridge);

			// Check that the network bridge is not yet in the list
			if (it != m_bridges.end())
			{
				// The network bridge already exists in the list
				return false;
			}

			// Add the network bridge to the list
			m_bridges.push_back(bridge);
		}

		// The network bridge is added
		return true;
	}

	// Remove a network bridge from the list
	bool BridgeList::remove(Bridge* bridge)
	{
		// Check pointer to the network bridge
		if (bridge == nullptr)
		{
			return false;
		}

		{
			// Lock the list of network bridges
			std::lock_guard<std::mutex> lock(m_mtx);

			// Look for the network bridge in the list
			auto it = std::find(m_bridges.begin(), m_bridges.end(), bridge);

			// Check that the network bridge is exist in the list
			if (it == m_bridges.end())
			{
				// The network bridge doesn't exist in the list
				return false;
			}

			// Remove the network bridge from the list
			m_bridges.erase(it);
		}

		// The network bridge is removed
		return true;
	}

	// Clear the network bridges list
	void BridgeList::clear()
	{
		{
			// Lock the list of network bridges
			std::lock_guard<std::mutex> lock(m_mtx);

			// Clear
			m_bridges.clear();
		}
	}

	// Get a number of network bridges in the list
	std::size_t BridgeList::count() const
	{
		// Number of network bridges
		std::size_t number = 0;
		
		{
			// Lock the list of network bridges
			std::lock_guard<std::mutex> lock(m_mtx);

			// Get the number of network bridges
			number = m_bridges.size();
		}

		return number;
	}

	// Check that a network bridge exists in the list
	bool BridgeList::exists(Bridge* bridge) const
	{
		{
			// Lock the list of network bridges
			std::lock_guard<std::mutex> lock(m_mtx);

			// Look for the network bridge in the list
			auto it = std::find(m_bridges.begin(), m_bridges.end(), bridge);

			// Check that the network bridge is exist in the list
			if (it != m_bridges.end())
			{
				// The network bridge exists in the list
				return true;
			}
		}

		// The network bridge doesn't exist in the list
		return false;
	}

	// Execute a custom function over each network bridge in the list
	void BridgeList::for_each(std::function<void(Bridge&, bool&)> func)
	{
		{
			// Lock the list of network bridges
			std::lock_guard<std::mutex> lock(m_mtx);

			// Loop stop flag
			bool stop = false;

			// Go through each element
			for (auto bridge : m_bridges)
			{
				// Call the function on a network bridge
				func(*bridge, stop);

				// Check that it is necessary to stop the execution of the loop
				if (stop == true)
				{
					// Exit the loop
					break;
				}
			}
		}
	}

	// Send a packet to all network bridges in the list
	void BridgeList::send_to_all(const IPacket& packet, endpoint_type_t direction) const
	{
		{
			// Lock the list of network bridges
			std::lock_guard<std::mutex> lock(m_mtx);

			// Go through each element
			for (auto bridge : m_bridges)
			{
				// Determine the direction of transmission
				switch (direction)
				{
					// Send the packet to the Game.exe
					case endpoint_type_t::game:
						{
							bridge->send_packet_game(packet);
						}
						break;

					// Send the packet to the GateServer.exe
					case endpoint_type_t::gate:
						{
							bridge->send_packet_gate(packet);
						}
						break;
				}
			}
		}
	}

	// Find a network bridge by user condition
	std::optional<Bridge*> BridgeList::find(std::function<bool(Bridge&)> func)
	{
		{
			// Lock the list of network bridges
			std::lock_guard<std::mutex> lock(m_mtx);

			// Look for a network bridge by user condition
			auto it = std::find_if(m_bridges.begin(), m_bridges.end(), 
				[&func](Bridge* bridge)
				{
					// Call user condition function
					return func(*bridge);
				}
			);

			// Check that the network bridge is found
			if (it != m_bridges.end())
			{
				// Network bridge is found
				return { (*it) };
			}
		}

		// Bridges not found
		return std::nullopt;
	}

	// Find a network bridge by account
	std::optional<Bridge*> BridgeList::find_by_account(const std::string& account)
	{
		return find(
			[&account](Bridge& bridge) -> bool
			{
				return (
					utils::string::lower_case(bridge.player().login) 
						== utils::string::lower_case(account)
				);
			}
		);
	}

	// Find a network bridge by character
	std::optional<Bridge*> BridgeList::find_by_character(const std::string& character)
	{
		return find(
			[&character](Bridge& bridge) -> bool
			{
				return (
					utils::string::lower_case(bridge.player().cha_name)
						== utils::string::lower_case(character)
				);
			}
		);
	}
}