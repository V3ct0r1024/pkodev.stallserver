#include "PacketHandlerStorage.h"

namespace pkodev
{
	// Constructor 
	PacketHandlerStorage::PacketHandlerStorage()
	{

	}

	// Destructor
	PacketHandlerStorage::~PacketHandlerStorage()
	{
		// Remove all packet handlers and free memory
		m_handlers.clear();
	}

	// Add packet handler
	void PacketHandlerStorage::add_handler(handler_ptr_t&& handler)
	{
		// Check that the handler does not exist in the repository yet
		if (check_handler(handler->id()) == false)
		{
			// Add the handler to the repository
			m_handlers.insert(
				std::make_pair<unsigned short int, handler_ptr_t>(
					handler->id(), std::move(handler)
				)
			);
		}
	}

	// Remove packet handler from repository by pointer
	void PacketHandlerStorage::remove_handler(handler_ptr_t& handler)
	{
		remove_handler( handler->id() );
	}

	// Remove packet handler from repository by packet ID
	void PacketHandlerStorage::remove_handler(unsigned short int packet_id)
	{
		// Look for the handler in the repository
		auto it = m_handlers.find(packet_id);

		// Check that the handler was found
		if (it != m_handlers.end())
		{
			// Remove the handler from the repository
			m_handlers.erase(it);
		}
	}

	// Get packet handler by packet ID
	const handler_ptr_t& PacketHandlerStorage::get_handler(unsigned short int packet_id) const
	{
		// Look for the handler in the repository
		auto it = m_handlers.find(packet_id);

		// Check that the handler was found
		if (it == m_handlers.end())
		{
			throw packet_handler_storage_exception(
				std::string(
					"Packet with ID" + std::to_string(packet_id) + " not found in the packet repository!"
				)
			);
		}

		return it->second;
	}

	// Check that packet handler exists in the repository
	bool PacketHandlerStorage::check_handler(unsigned short int packet_id) const
	{
		return ( m_handlers.find(packet_id) != m_handlers.end() );
	}
}