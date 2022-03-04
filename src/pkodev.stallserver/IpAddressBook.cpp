#include "IpAddressBook.h"

namespace pkodev
{
	// Constructor
	IpAddressBook::IpAddressBook()
	{

	}

	// Destructor
	IpAddressBook::~IpAddressBook()
	{

	}

	// Add an IP address to the list
	void IpAddressBook::register_ip(const std::string& ip)
	{
		{
			// Lock the IP address list
			std::lock_guard<std::mutex> lock(m_addresses_mtx);

			// Look for an IP address in the list
			auto it = m_addresses.find(ip);

			// Check that the IP address is found
			if (it != m_addresses.end())
			{
				// Increase the number of connections from the IP address
				it->second.count++;

				// Update the connection time
				it->second.last_time = std::chrono::system_clock::now();
			}
			else
			{
				// Add the IP address to the list
				m_addresses[ip] = { 1, std::chrono::system_clock::now() };
			}
		}
	}

	// Remove an IP address from the list
	void IpAddressBook::unregister_ip(const std::string& ip, unsigned long long interval)
	{
		{
			// Lock the IP address list
			std::lock_guard<std::mutex> lock(m_addresses_mtx);

			// Look for an IP address in the list
			auto it = m_addresses.find(ip);

			// Check that the address is found
			if (it != m_addresses.end())
			{
				// Decrease the number of connections from the IP address
				it->second.count--;

				// Check that the time interval has passed since the last connection
				bool expired = ((it->second.last_time + std::chrono::milliseconds(interval))
					< std::chrono::system_clock::now());

				// If the connections number become 0 and the interval has expired,
				//  then remove the IP address from the list
				if (it->second.count == 0 && expired == true)
				{
					m_addresses.erase(it);
				}
			}
		}
	}

	// Clear the IP address list
	void IpAddressBook::clear()
	{
		{
			// Lock the IP address list
			std::lock_guard<std::mutex> lock(m_addresses_mtx);

			// Clear
			m_addresses.clear();
		}
	}

	// Get the number of used identical IP addresses
	std::size_t IpAddressBook::get_ip_count(const std::string& ip) const
	{
		{
			// Lock the IP address list
			std::lock_guard<std::mutex> lock(m_addresses_mtx);

			// Look for an IP address in the list
			auto it = m_addresses.find(ip);

			// Check that the address is found
			if (it != m_addresses.end())
			{
				// Return the number of connections from the IP address
				return it->second.count;
			}
		}

		// IP address not found
		return 0;
	}

	// Check that some time in ms has passed since the last connection from the specified IP address
	bool IpAddressBook::is_time_expired(const std::string& ip, unsigned long long interval) const
	{
		{
			// Lock the IP address list
			std::lock_guard<std::mutex> lock(m_addresses_mtx);

			// Look for an IP address in the list
			auto it = m_addresses.find(ip);

			// Check that the address is found
			if (it != m_addresses.end())
			{
				// Check that the time interval has passed since the last connection
				return (
					( it->second.last_time + std::chrono::milliseconds(interval) )
						< std::chrono::system_clock::now()
				);
			}
		}

		// IP address not found
		return true;
	}
}