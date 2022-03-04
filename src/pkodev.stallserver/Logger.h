#pragma once
#include "Utils.h"

#include <string>
#include <fstream>
#include <mutex>

namespace pkodev
{
	// Class for logging events
	class Logger final
	{
		public:

			// Get an instance of the class (Meyer's singleton implementation)
			static Logger& Instance();

			// Destructor
			~Logger();

			// Open log file
			bool open(const std::string& path);
			
			// Close log file
			void close();

			// Add a message to the log
			template<typename... Args>
			void log(const std::string format, Args... args);

		private:

			// Constructor
			Logger();

			// Copy constructor
			Logger(const Logger&) = delete;

			// Move constructor
			Logger(Logger&&) = delete;

			// Copy assignment operator
			Logger& operator=(const Logger&) = delete;

			// Move assignment operator
			Logger& operator=(Logger&&) = delete;

			// Format a string
			std::string str_format(const std::string format, ...) const;

			// Write a message to debug window
			void debug(const std::string& message) const;


			// Output file stream mutex
			std::mutex m_file_mtx;

			// Output file stream
			std::ofstream m_file_stream;
	};


	// Add a message to the log
	template<typename... Args>
	void Logger::log(const std::string format, Args... args)
	{
		// Get current time
		tm timeinfo = utils::time::system_time();

		// Build log line
		std::string message = str_format(
			std::string(
				std::string("[%04X | %02d:%02d:%02d] ") +
				format +
				'\n'
			),
			static_cast<unsigned int>(GetCurrentThreadId()),
			timeinfo.tm_hour,
			timeinfo.tm_min,
			timeinfo.tm_sec,
			std::forward<Args>(args)...
		);

		// Output the line to log file
		{
			// Lock the output file stream
			std::lock_guard<std::mutex> lock(m_file_mtx);

			// Check if the file stream is open
			if (m_file_stream.is_open() == true)
			{
				// Write the log line to the file and flush it
				m_file_stream << message;
				m_file_stream.flush();
			}

			// Write the log line to debug window
			debug(message);
		}
	}
}


