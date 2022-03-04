#pragma once
#include <string>
#include <map>
#include <chrono>
#include <mutex>

namespace pkodev
{
	// Class for storing information about connected IP addresses
	class IpAddressBook final
	{
		public:

			// Constructor
			IpAddressBook();

			// Copy constructor
			IpAddressBook(const IpAddressBook&) = delete;

			// Move constructor
			IpAddressBook(IpAddressBook&&) = delete;

			// Destructor 
			~IpAddressBook();

			// Copy assignment operator
			IpAddressBook& operator=(const IpAddressBook&) = delete;

			// Move assignment operator
			IpAddressBook& operator=(IpAddressBook&&) = delete;

			// Add an IP address to the list
			void register_ip(const std::string& ip);

			// Remove an IP address from the list
			void unregister_ip(const std::string& ip, unsigned long long interval);

			// Clear the IP address list
			void clear();

			// Get the number of used identical IP addresses
			std::size_t get_ip_count(const std::string& ip) const;

			// Check that some time in ms has passed since the last connection from the specified IP address
			bool is_time_expired(const std::string& ip, unsigned long long interval) const;

		private:

			// Structure with IP address data
			struct ip_address_data final
			{
				// Connection number
				std::size_t count;

				// Last connection time
				std::chrono::time_point<std::chrono::system_clock> last_time;

				// Constructor
				ip_address_data() :
					count(0), last_time(std::chrono::system_clock::now()) { }

				// Constructor
				ip_address_data(std::size_t _count, std::chrono::time_point<std::chrono::system_clock> _time) :
					count(_count), last_time(_time) { }
			};

			// List of connected IP addresses
			std::map<std::string, ip_address_data> m_addresses;

			// Mutex for protecting the list of IP addresses
			mutable std::mutex m_addresses_mtx;
	};
}

