#pragma once
#include "Packet.h"

#include <set>
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
	typedef std::set<Bridge*> bridge_list_t;

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
			bool add(const Bridge* bridge);

			// Remove a network bridge from the list
			bool remove(const Bridge* bridge);

			// Clear the network bridges list
			void clear();

			// Get a number of network bridges in the list
			std::size_t count() const;

			// Check that a network bridge exists in the list
			bool exists(const Bridge* bridge) const;

			// Execute a custom function over each network bridge in the list
			void for_each(std::function<bool(Bridge&)> func);

			// Send a packet to all network bridges in the list
			void send_to_all(const IPacket& packet, endpoint_type_t direction);

			// Find a network bridge by user condition
			std::optional<Bridge*> find(std::function<bool(const Bridge&)> condition) const;

			// Find a network bridge by account
			std::optional<Bridge*> find_by_account(const std::string& account) const;

			// Find a network bridge by character
			std::optional<Bridge*> find_by_character(const std::string& character) const;

		private:

			// List of network bridges
			bridge_list_t m_bridges;

			// Mutex for protecting the list of network bridges
			mutable std::mutex m_mtx;
	};

}
