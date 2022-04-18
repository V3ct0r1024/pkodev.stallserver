#include "BridgeList.h"
#include "Bridge.h"
#include "Utils.h"

namespace pkodev
{
	// Constructor
	BridgeList::BridgeList()
	{

	}

	// Destructor
	BridgeList::~BridgeList()
	{

	}

	// Add a network bridge to the list
	bool BridgeList::add(const Bridge* bridge)
	{
		// Check a network bridge pointer
		if (bridge == nullptr)
		{
			return false;
		}

		// Lock the list of network bridges
		std::lock_guard<std::mutex> lock(m_mtx);

		// Add the network bridge to the list
		auto ret = m_bridges.insert(const_cast<Bridge*>(bridge));

		// Return the insertion result
		return ret.second;
	}

	// Remove a network bridge from the list
	bool BridgeList::remove(const Bridge* bridge)
	{
		// Check a network bridge pointer
		if (bridge == nullptr)
		{
			return false;
		}

		// Lock the list of network bridges
		std::lock_guard<std::mutex> lock(m_mtx);

		// Remove the network bridge from the list
		std::size_t count = m_bridges.erase(const_cast<Bridge*>(bridge));

		// Check that bridge has been removed
		if (count == 0)
		{
			// The bridge was not on the list
			return false;
		}

		// The network bridge is removed
		return true;
	}

	// Clear the network bridges list
	void BridgeList::clear()
	{
		// Lock the list of network bridges
		std::lock_guard<std::mutex> lock(m_mtx);

		// Clear the list
		m_bridges.clear();
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
	bool BridgeList::exists(const Bridge* bridge) const
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

		// The network bridge doesn't exist in the list
		return false;
	}

	// Execute a custom function over each network bridge in the list
	void BridgeList::for_each(std::function<bool(Bridge&)> func)
	{
		// Lock the list of network bridges
		std::lock_guard<std::mutex> lock(m_mtx);

		// Loop stop flag
		bool stop = false;

		// Go through each element
		for (auto bridge : m_bridges)
		{
			{
				// Lock the current bridge
				std::lock_guard<std::recursive_mutex> lock(bridge->get_lock());

				// Call the function on the network bridge
				stop = func(*bridge);
			}

			// Check that it is necessary to stop the execution of the loop
			if (stop == true)
			{
				// Exit the loop
				break;
			}
		}
	}

	// Send a packet to all network bridges in the list
	void BridgeList::send_to_all(const IPacket& packet, endpoint_type_t direction)
	{
		for_each(
			[&](Bridge& other) -> bool
			{
				// Determine the direction of transmission
				switch (direction)
				{
					// Send the packet to the Game.exe
					case endpoint_type_t::game: other.send_packet_game(packet); break;

					// Send the packet to the GateServer.exe
					case endpoint_type_t::gate: other.send_packet_gate(packet); break;
				}

				return false;
			}
		);
	}

	// Find a network bridge by user condition
	std::optional<Bridge*> BridgeList::find(std::function<bool(const Bridge&)> func) const
	{
		// Lock the list of network bridges
		std::lock_guard<std::mutex> lock(m_mtx);

		// Look for a network bridge by user condition
		auto it = std::find_if(m_bridges.begin(), m_bridges.end(), 
			[&func](const Bridge* bridge) -> bool
			{
				// Is a bridge meets conditions?
				bool ret = false;

				{
					// Lock the bridge
					std::lock_guard<std::recursive_mutex> lock(bridge->get_lock());

					// Call user condition function
					ret = func(*bridge);
				}

				return ret;
			}
		);

		// Check that the network bridge is found
		if (it != m_bridges.end())
		{
			// Network bridge is found
			return { (*it) };
		}

		// Bridges not found
		return std::nullopt;
	}

	// Find a network bridge by account
	std::optional<Bridge*> BridgeList::find_by_account(const std::string& account) const
	{
		return find(
			[&account](const Bridge& bridge) -> bool
			{
				return (
					utils::string::lower_case(bridge.player().login) 
						== utils::string::lower_case(account)
				);
			}
		);
	}

	// Find a network bridge by character
	std::optional<Bridge*> BridgeList::find_by_character(const std::string& character) const
	{
		return find(
			[&character](const Bridge& bridge) -> bool
			{
				return (
					utils::string::lower_case(bridge.player().cha_name)
						== utils::string::lower_case(character)
				);
			}
		);
	}
}