#pragma once
#include <string>
#include <ctime>
#include <WinSock2.h>
#include <WS2tcpip.h>


namespace pkodev
{
	namespace utils
	{
		// Network functions
		class network
		{
			public:

				// Get IP address from structure sockaddr
				static std::string get_ip_address(sockaddr* address);

				// Get port from structure sockaddr
				static unsigned short int get_port(sockaddr* address);

				// Get pending error on the socket
				static int get_socket_error(SOCKET fd);
		};

		// Data validation functions
		class validation
		{
			public:

				// Check MAC address
				static bool check_mac_address(const std::string& mac_address);

				// Check MD5 hash string
				static bool check_md5_hash(const std::string& md5_hash);

		};

		// String manipulation functions
		class string
		{
			public:

				// Convert string to lowercase
				static std::string lower_case(std::string str);

				// Convert string to uppercase
				static std::string upper_case(std::string str);

				// Remove leading whitespace from a string
				static std::string left_trim(const std::string& str, const std::string& whitespace = " \t");
				
				// Remove trailing whitespace from a string
				static std::string right_trim(const std::string& str, const std::string& whitespace = " \t");

				// Remove leading and trailing whitespace from a string
				static std::string trim(const std::string& str, const std::string& whitespace = " \t");

		};

		// File functions
		class file
		{
			public:

				// Extract filename from path
				static std::string extract_filename(const std::string& path);

				// Extract directory from path
				static std::string extract_filepath(const std::string& path);
				
				// Change file extension
				static std::string change_fileext(const std::string& path, const std::string& ext);

		};

		// Time functions
		class time
		{
			public:

				// Get the current system time
				static tm system_time();

				// Convert time in milliseconds to a timeval structure
				static timeval timeval_from_msec(unsigned long long milliseconds);

		};
	}
}

