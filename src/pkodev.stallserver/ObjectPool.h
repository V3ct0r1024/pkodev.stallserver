#pragma once
#include <list>
#include <memory>
#include <mutex>
#include <stdexcept>

namespace pkodev
{
	// Interface of object that can be put into an object pool
	class IPoolable
	{
		public:

			// Constructor
			IPoolable() = default;

			// Destructor
			virtual ~IPoolable() = default;

			// Reset object state
			virtual void reset() = 0;

	};

	// Object creator interface
	class IPoolableMaker
	{
		public:

			// Constructor
			IPoolableMaker() = default;

			// Destructor
			virtual ~IPoolableMaker() = default;

			// Create an object
			virtual IPoolable* create() = 0;

	};

	// Object pool class exception
	class object_pool_exception final : public std::runtime_error
	{
		public:

			// Constructor with const char *
			object_pool_exception(const char* what) :
				std::runtime_error(what) { }

			// Constructor with std::string
			object_pool_exception(const std::string& what) :
				std::runtime_error(what) { }
	};

	// Object pool class
	class CObjectPool final
	{
		public:

			// Constructor
			CObjectPool(std::unique_ptr<IPoolableMaker>&& maker, std::size_t size);

			// Copy constructor
			CObjectPool(const CObjectPool&) = delete;

			// Move constructor
			CObjectPool(CObjectPool&&) = delete;

			// Destructor 
			~CObjectPool();

			// Copy assignment operator
			CObjectPool& operator=(const CObjectPool&) = delete;

			// Move assignment operator
			CObjectPool& operator=(CObjectPool&&) = delete;

			// Take an object from the pool
			IPoolable* acquire();

			// Return an object to the pool
			void release(IPoolable* object);

			// Get object pool size
			inline std::size_t size() const { return m_pool_size; }

			// Get number of objects in the object pool that can be taken
			std::size_t available() const;

			// Check if there are objects in the pool
			inline bool is_empty() const { return (available() == 0); }

		private:

			// Pool size
			std::size_t m_pool_size;

			// Object creator
			std::unique_ptr<IPoolableMaker> m_object_maker;

			// List of objects
			std::list<IPoolable*> m_pool;

			// Mutex for protecting the list of objects
			mutable std::mutex m_pool_mtx;
	};

}

