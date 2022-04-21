#include "Utils.h"
#include <algorithm>

namespace pkodev
{
	// Get IP address from structure sockaddr
	std::string utils::network::get_ip_address(sockaddr* address)
	{
		// A buffer for IP address
		char buffer[16]{0x00};
		
		// Extract IP address from sockaddr structure
		PCSTR ret = inet_ntop(
			AF_INET, 
			&(reinterpret_cast<sockaddr_in*>(address)->sin_addr), 
			buffer,
			sizeof(buffer)
		);

		// Check inet_ntop() function result
		if (ret != nullptr)
		{
			// IP address extracted
			return std::string(buffer);
		}

		// Return an empty string
		return std::string("");
	}

	// Get port from structure sockaddr
	unsigned short int utils::network::get_port(sockaddr* address)
	{
		// Check the address family IPv4
		if (address->sa_family == AF_INET)
		{
			return ntohs(reinterpret_cast<sockaddr_in*>(address)->sin_port);
		}

		// Unknown address family
		return 0;
	}

	// Get pending error on the socket
	int utils::network::get_socket_error(SOCKET fd)
	{
		// Error code
		int errcode = 0;

		// Error code data size
		int length = sizeof(errcode);

		// Get socket error code
		const int ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&errcode), &length);

		// Check getsockopt() function
		if (ret == SOCKET_ERROR)
		{
			// Error
			return WSAGetLastError();
		}

		// Return socket error code
		return errcode;
	}

	// Convert IP address from string to integer form
	unsigned int utils::network::ip_address_to_int(const std::string& ip_address)
	{
		// The result
		unsigned int ret = 0;
		
		// Convert IP address
		if ( inet_pton(AF_INET, ip_address.c_str(), reinterpret_cast<void*>(&ret)) != 1 )
		{
			// Error, invalid IP address
			return 0; // 0.0.0.0
		}

		return ret;
	}

	// Check MAC address
	bool utils::validation::check_mac_address(const std::string& mac_address)
	{
		// Check length of the string with MAC address
		if (mac_address.length() != 23)
		{
			// Incorrect length
			return false;
		}

		// Check every character in the string
		for (unsigned int i = 0; i < mac_address.length(); ++i)
		{
			// Take current character
			const unsigned char c = static_cast<unsigned char>(mac_address[i]);

			// Every 3rd character must be a hyphen (-)
			if ( ( (i + 1) % 3 == 0 ) && (c != '-') )
			{
				// Incorrect MAC format
				return false;
			}

			// The rest of the characters can take on the values
			// 0 - 1
			// A - F
			if ( ((i + 1) % 3 != 0) && ( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ) == false )
			{
				// Incorrect MAC format
				return false;
			}
		}

		// MAC address verified
		return true;
	}

	// Check MD5 hash string
	bool utils::validation::check_md5_hash(const std::string& md5_hash)
	{
		// Check hash length
		if (md5_hash.length() != 32)
		{
			// Incorrect length
			return false;
		}

		// // Check every character in the string
		for (unsigned int i = 0; i < md5_hash.length(); ++i)
		{
			// Take current character
			const unsigned char c = static_cast<unsigned char>(md5_hash[i]);

			// Characters can take on the values
			// 0 - 1
			// A - F
			if ( ( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ) == false )
			{
				// Incorrect MD5 hash format
				return false;
			}
		}

		// MD5 hash verified
		return true;
	}


	// Convert string to lowercase
	std::string utils::string::lower_case(std::string str)
	{
		// Convert the string
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);

		// Return the converted string
		return str;
	}

	// Convert string to uppercase
	std::string utils::string::upper_case(std::string str)
	{
		// Convert the string
		std::transform(str.begin(), str.end(), str.begin(), ::toupper);

		// Return the converted string
		return str;
	}

	// Remove leading whitespace from a string
	std::string utils::string::left_trim(const std::string& str, const std::string& whitespace)
	{
		// Looking for whitespaces at the beginning of a string
		const std::size_t pos = str.find_first_not_of(whitespace);

		// Remove spaces at the beginning of a string
		if (std::string::npos != pos) 
		{
			return str.substr(pos);
		}

		return str;
	}

	// Remove trailing whitespace from a string
	std::string utils::string::right_trim(const std::string& str, const std::string& whitespace)
	{
		// Looking for whitespaces at the end of a string
		const std::size_t pos = str.find_last_not_of(whitespace);

		// Remove spaces at the end of a string
		if (std::string::npos != pos)
		{
			return str.substr(0, pos + 1);
		}

		return str;
	}

	// Remove leading and trailing whitespace from a string
	std::string utils::string::trim(const std::string& str, const std::string& whitespace)
	{
		return left_trim(right_trim(str, whitespace), whitespace);
	}

	// Extract filename from path
	std::string utils::file::extract_filename(const std::string& path)
	{
		// Looking for last slash
		const std::size_t pos = path.find_last_of("/\\");

		// Check that the slash is found
		if (pos != std::string::npos)
		{
			// Extract filename
			return path.substr(pos + 1);
		}

		return path;
	}

	// Extract directory from path
	std::string utils::file::extract_filepath(const std::string& path)
	{
		// Looking for last slash
		const std::size_t pos = path.find_last_of("/\\");

		// Check that the slash is found
		if (pos != std::string::npos)
		{
			// Extract directory
			return path.substr(0, pos);
		}

		return path;
	}

	// Change file extension
	std::string utils::file::change_fileext(const std::string& path, const std::string& ext)
	{
		// Looking for an extension dot
		const std::size_t pos = path.find_last_of('.');

		// Check that the extension dot is found
		if (pos != std::string::npos)
		{
			return path.substr(0, pos + 1) + ext;
		}

		// Could not change extension
		return path;
	}

	// Get the current system time
	tm utils::time::system_time()
	{
		// Get the current system time
		time_t rawtime = 0;
		::time(&rawtime);

		// Fill the time structure
		tm timeinfo;
		::localtime_s(&timeinfo, &rawtime);

		// Add '1900' to the year
		timeinfo.tm_year += 1900;

		// Add '1' to the month
		timeinfo.tm_mon += 1;

		return timeinfo;
	}

	// Convert time in milliseconds to a timeval structure
	timeval utils::time::timeval_from_msec(unsigned long long milliseconds)
	{
		timeval ret;
		const unsigned long long microseconds = milliseconds * 1000;

		ret.tv_sec = static_cast<long>(microseconds / 1000000);
		ret.tv_usec = static_cast<long>(microseconds % 1000000);

		return ret;
	}
}