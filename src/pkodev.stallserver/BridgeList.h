#pragma once
#include "Packet.h"

#include <vector>
#include <mutex>
#include <functional>
#include <optional>

namespace pkodev
{
	// Network bridge class forward declaration
	class Bridge;

	// Determine the type of side of network bridge
	enum class endpoint_type_t : unsigned short int;

	// Determine the storage type for network bridges
	typedef std::vector<Bridge*> bridge_list_t;

	// List of network bridges
	class BridgeList final
	{
		public:

			// Constructor
			BridgeList();

			// Copy constructor
			BridgeList(const BridgeList&) = delete;

			// Move constructor
			BridgeList(BridgeList&&) = delete;

			// Destructor 
			~BridgeList();

			// Copy assignment operator
			BridgeList& operator=(const BridgeList&) = delete;

			// Move assignment operator
			BridgeList& operator=(BridgeList&&) = delete;

			// Add a network bridge to the list
			bool add(Bridge* bridge);

			// Remove a network bridge from the list
			bool remove(Bridge* bridge);

			// Clear the network bridges list
			void clear();

			// Get a number of network bridges in the list
			std::size_t count() const;

			// Check that a network bridge exists in the list
			bool exists(Bridge* bridge) const;

			// Execute a custom function over each network bridge in the list
			void for_each(std::function<void(Bridge&, bool&)> func);

			// Send a packet to all network bridges in the list
			void send_to_all(const IPacket& packet, endpoint_type_t direction) const;

			// Find a network bridge by user condition
			std::optional<Bridge*> find(std::function<bool(Bridge&)> condition);

			// Find a network bridge by account
			std::optional<Bridge*> find_by_account(const std::string& account);

			// Find a network bridge by character
			std::optional<Bridge*> find_by_character(const std::string& character);

		private:

			// List of network bridges
			bridge_list_t m_bridges;

			// Mutex for protecting the list of network bridges
			mutable std::mutex m_mtx;
	};

}
