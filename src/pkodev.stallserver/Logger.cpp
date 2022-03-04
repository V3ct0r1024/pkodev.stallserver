#include "Logger.h"

#include <cstdarg>
#include <cstdio>
#include <ctime>

#include <Windows.h>

namespace pkodev
{
	// Get an instance of the class (Meyer's singleton implementation)
	Logger& Logger::Instance()
	{
		static Logger instance;
		return instance;
	}

	// Constructor
	Logger::Logger()
	{

	}

	// Destructor
	Logger::~Logger()
	{
		// Close log
		close();
	}

	// Open log file
	bool Logger::open(const std::string& path)
	{
		// A check if log file is open
		bool is_open = false;

		// Open log file
		{
			// Lock the output file stream
			std::lock_guard<std::mutex> lock(m_file_mtx);

			// Check if the file stream is open
			if (m_file_stream.is_open() == true)
			{
				// Close the file stream
				m_file_stream.close();
			}

			// Open the file stream again
			m_file_stream.open(path, std::ios::app);

			// Get state of the file stream
			is_open = m_file_stream.is_open();
		}

		// Check that the log file is open
		if (is_open == true)
		{
			// Write a message to the log that logging has been started
			log("Logging has been started.");
		}
		else
		{
			// Could not open the log file
			debug("Could not open the log file " + path + "!");
		}

		// Return the result
		return is_open;
	}

	// Close log file
	void Logger::close()
	{
		// A check if log file is open
		bool is_open = false;

		// Get state of the file stream
		{
			// Lock the output file stream
			std::lock_guard<std::mutex> lock(m_file_mtx);

			// Get state of the file stream
			is_open = m_file_stream.is_open();
		}

		// Check if logging is enabled now
		if (is_open == true)
		{
			// Write a message to the log that logging has been stopped
			log("Logging has been stopped.");

			// Close the log
			{
				// Lock the output file stream
				std::lock_guard<std::mutex> lock(m_file_mtx);

				// Close the file stream
				m_file_stream.close();
			}
		}
	}

	// Format a string
	std::string Logger::str_format(const std::string format, ...) const
	{
		// Buffer for formatted string
		char buf[1024]{0x00};

		// Format the string
		va_list args;
		va_start(args, format);
			_vsnprintf_s(buf, sizeof(buf), _TRUNCATE, format.c_str(), args);
		va_end(args);

		// Return formatted string
		return std::string(buf);
	}

	// Write a message to debug window
	void Logger::debug(const std::string& message) const
	{
		#if defined (_WIN32) && defined(_DEBUG)
			OutputDebugStringA(message.c_str());
		#endif
	}
}