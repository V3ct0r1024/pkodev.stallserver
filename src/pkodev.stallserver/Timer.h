#pragma once
#include <string>
#include <functional>

#include <event2/event.h>

namespace pkodev
{
	// Timer class
	class Timer final
	{
		public:

			// Constructor
			Timer(event_base* ev_base, const std::string& name);

			// Copy constructor
			Timer(const Timer&) = delete;

			// Move constructor
			Timer(Timer&&) = delete;

			// Destructor 
			~Timer();

			// Copy assignment operator
			Timer& operator=(const Timer&) = delete;

			// Move assignment operator
			Timer& operator=(Timer&&) = delete;

			// Define a user function that will be called when the timer expires
			void on_timer(std::function<void()> callback);

			// Start the timer
			bool start(unsigned long long milliseconds);

			// Stop the timer
			bool stop();

			// Get the timer name
			inline const std::string& name() const { return m_name; }

			// Is the timer started
			inline bool enabled() const { return m_enabled; }

			// Get the timer interval
			inline unsigned long long interval() const { return m_interval_ms; }

		private:

			// Timer event handler
			void handle_event();


			// Timer name
			std::string m_name;

			// Timer event
			event* m_event;

			// User function to be called when the timer expires
			std::function<void()> m_callback;

			// Is the timer started
			bool m_enabled;

			// Timer interval in milliseconds
			unsigned long long m_interval_ms;
	};
}
