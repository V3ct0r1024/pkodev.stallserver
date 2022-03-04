#include "Timer.h"
#include "Logger.h"
#include "Utils.h"

namespace pkodev
{
	// Constructor
	Timer::Timer(event_base* ev_base, const std::string& name) :
		m_name(name),
		m_callback({}),
		m_enabled(false),
		m_interval_ms(0)
	{
		// Create timer event
		m_event = evtimer_new(
			ev_base,
			[](evutil_socket_t fd, short int what, void* ctx) noexcept
			{
				// Pointer to the timer object
				Timer* timer = reinterpret_cast<Timer*>(ctx);

				// Catch all exceptions
				try
				{
					// Call timer event handler
					timer->handle_event();
				}
				catch (...)
				{
					// Write a message to the log
					Logger::Instance().log("Caught an exception in '%s' timer event method!", timer->name().c_str());
				}
			},
			reinterpret_cast<void*>(this)
		);
	}

	// Destructor
	Timer::~Timer()
	{
		// Delete timer event
		if (m_event != nullptr)
		{
			event_free(m_event);
			m_event = nullptr;
		}
	}

	// Define a user function that will be called when the timer expires
	void Timer::on_timer(std::function<void()> callback)
	{
		// Check that the timer is not running
		if (m_enabled == false)
		{
			// Update user function
			m_callback = callback;
		}
		else
		{
			// Write a message to the log
			Logger::Instance().log("Timer '%s': Couldn't define user function because the timer is running!", m_name.c_str());
		}
	}

	// Start the timer
	bool Timer::start(unsigned long long milliseconds)
	{
		// Check that the timer is not running
		if (m_enabled == true)
		{
			// Write a message to the log
			Logger::Instance().log("Can't start timer '%s' because timer is already started!", m_name.c_str());

			// Timer is already started
			return false;
		}

		// Check that timer event was created
		if (m_event == nullptr)
		{
			// Write a message to the log
			Logger::Instance().log("Can't start timer '%s' because event is null!", m_name.c_str());

			// Timer event not created
			return false;
		}

		// Check that user function is defined
		if (m_callback == nullptr)
		{
			// Write a message to the log
			Logger::Instance().log("Can't start timer '%s' because user function is not defined!", m_name.c_str());

			// User function is not defined
			return false;
		}
			
		// Convert time in milliseconds to timeval structure
		timeval tm = utils::time::timeval_from_msec(milliseconds);

		// Add timer event to the event loop
		int ret = evtimer_add(m_event, &tm);

		// Check result
		if (ret != 0)
		{
			// Write a message to the log
			Logger::Instance().log("Can't start timer '%s' because evtimer_add() failed!", m_name.c_str());

			// Timer could not be started because the evtimer_add() function returned an error
			return false;
		}

		// Raise timer state flag
		m_enabled = true;

		// Update timer interval
		m_interval_ms = milliseconds;

		// Timer is started
		return true;
	}

	// Stop the timer
	bool Timer::stop()
	{
		// Check that the timer is running
		if (m_enabled == false)
		{
			// Write a message to the log
			Logger::Instance().log("Can't stop timer '%s' because it is already stopped!", m_name.c_str());

			// Timer is already stopped
			return false;
		}

		// Remove timer event from the event loop
		int ret = evtimer_del(m_event);

		// Check result
		if (ret != 0)
		{
			// Write a message to the log
			Logger::Instance().log("Can't stop timer '%s' because evtimer_del() failed!", m_name.c_str());

			// Timer could not be stopped because the evtimer_del() function returned an error
			return false;
		}

		// Reset timer state flag
		m_enabled = false;

		// Reset timer interval
		m_interval_ms = 0;

		// Timer is stopped
		return true;
	}

	// Timer event handler
	void Timer::handle_event()
	{
		// Reset timer state flag
		m_enabled = false;

		// Reset timer interval
		m_interval_ms = 0;

		// Check that user function has been defined
		if (m_callback)
		{
			// Call user function
			m_callback();
		}
		else
		{
			// Write a message to the log
			Logger::Instance().log("Timer '%s': User function is not defined!", m_name.c_str());
		}
	}
}