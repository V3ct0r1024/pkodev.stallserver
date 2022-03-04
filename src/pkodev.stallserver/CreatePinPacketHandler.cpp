#include "CreatePinPacketHandler.h"
#include "RingBuffer.h"
#include "Bridge.h"
#include "Utils.h"

namespace pkodev
{
	// Constructor
	CreatePinPacketHandler::CreatePinPacketHandler() :
		m_pin("")
	{

	}

	// Destructor
	CreatePinPacketHandler::~CreatePinPacketHandler()
	{

	}

	// Packet ID
	unsigned short int CreatePinPacketHandler::id() const
	{
		return 346;
	}

	// Packet name
	std::string CreatePinPacketHandler::name() const
	{
		return std::string("Create pin");
	}

	// Transmission direction
	packet_direction_t CreatePinPacketHandler::direction() const
	{
		return packet_direction_t::cs;
	}

	// Read packet
	void CreatePinPacketHandler::read(std::size_t packet_size, LinearBuffer& buffer)
	{
		m_pin = static_cast<std::string>(buffer.read_string());
	}

	// Handle packet
	bool CreatePinPacketHandler::handle(Bridge& bridge)
	{
		// Pass the packet further
		return true;
	}

	// Validate packet
	bool CreatePinPacketHandler::validate() const
	{
		// Check MD5 hash of pin code
		return utils::validation::check_md5_hash(m_pin);
	}
}