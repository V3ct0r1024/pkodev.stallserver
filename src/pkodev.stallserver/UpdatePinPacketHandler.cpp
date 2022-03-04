#include "UpdatePinPacketHandler.h"
#include "RingBuffer.h"
#include "Bridge.h"
#include "Utils.h"

namespace pkodev
{
	// Constructor
	UpdatePinPacketHandler::UpdatePinPacketHandler() :
		m_old_pin(""),
		m_new_pin("")
	{

	}

	// Destructor
	UpdatePinPacketHandler::~UpdatePinPacketHandler()
	{

	}

	// Packet ID
	unsigned short int UpdatePinPacketHandler::id() const
	{
		return 347;
	}

	// Packet name
	std::string UpdatePinPacketHandler::name() const
	{
		return std::string("Update pin");
	}

	// Transmission direction
	packet_direction_t UpdatePinPacketHandler::direction() const
	{
		return packet_direction_t::cs;
	}

	// Read packet
	void UpdatePinPacketHandler::read(std::size_t packet_size, LinearBuffer& buffer)
	{
		m_old_pin = static_cast<std::string>(buffer.read_string());
		m_new_pin = static_cast<std::string>(buffer.read_string());
	}

	// Handle packet
	bool UpdatePinPacketHandler::handle(Bridge& bridge)
	{
		// Pass the packet further
		return true;
	}

	// Validate packet
	bool UpdatePinPacketHandler::validate() const
	{
		// Check MD5 hashes of pin codes
		return ( (utils::validation::check_md5_hash(m_old_pin) == true) &&
			     (utils::validation::check_md5_hash(m_new_pin) == true) );
	}
}