#pragma once
#include "PacketHandler.h"

#include <string>
#include <map>
#include <memory>
#include <stdexcept>

namespace pkodev
{
	// Data type for storing a smart pointer to a packet handler
	typedef std::unique_ptr<IPacketHandler> handler_ptr_t;

	// Data type for storing packet handlers
	typedef std::map<unsigned short int, handler_ptr_t> handler_list_t;

	// Packet handler repository exception class
	class packet_handler_storage_exception final : public std::runtime_error
	{
		public:

			// Constructor with const char *
			packet_handler_storage_exception(const char* what) :
				std::runtime_error(what) { }

			// Constructor with std::string
			packet_handler_storage_exception(const std::string& what) :
				std::runtime_error(what) { }

	};

	// Packet handler repository
	class PacketHandlerStorage final
	{
		public:

			// Constructor
			PacketHandlerStorage();

			// Copy constructor
			PacketHandlerStorage(const PacketHandlerStorage&) = delete;

			// Move constructor
			PacketHandlerStorage(PacketHandlerStorage&&) = delete;

			// Destructor
			~PacketHandlerStorage();

			// Copy assignment operator
			PacketHandlerStorage& operator=(const PacketHandlerStorage&) = delete;

			// Move assignment operator
			PacketHandlerStorage& operator=(PacketHandlerStorage&&) = delete;

			// Add packet handler to the repository
			bool add_handler(handler_ptr_t&& handler);

			// Remove packet handler from repository by pointer
			void remove_handler(handler_ptr_t& handler);

			// Remove packet handler from repository by packet ID
			void remove_handler(unsigned short int packet_id);

			// Get packet handler by packet ID
			const handler_ptr_t& get_handler(unsigned short int packet_id) const;

			// Check that packet handler exists in the repository
			bool check_handler(unsigned short int packet_id) const;

		private:

			// List of packet handlers
			handler_list_t m_handlers;
	};
}

