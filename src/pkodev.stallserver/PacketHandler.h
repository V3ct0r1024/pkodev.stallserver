#pragma once
#include <string>

namespace pkodev
{
	// Byte buffer class forward declaration
	class LinearBuffer;

	// Network bridge class forward declaration
	class Bridge;

	// Transmission direction
	enum class packet_direction_t
	{
		cs = 0,    // C -> S
		sc         // S -> C
	};
	
	// Game logic layer network packet handler interface 
	class IPacketHandler
	{
		public:

			// Packet ID
			virtual unsigned short int id() const = 0;

			// Packet name
			virtual std::string name() const = 0;

			// Transmission direction
			virtual packet_direction_t direction() const = 0;

			// Read packet
			virtual void read(std::size_t, LinearBuffer&) = 0;

			// Handle packet
			virtual bool handle(Bridge&) = 0;

			// Validate packet
			virtual bool validate() const = 0; 
	};
}

