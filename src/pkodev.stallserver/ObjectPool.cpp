#include "ObjectPool.h"

namespace pkodev
{
	// Constructor
	CObjectPool::CObjectPool(std::unique_ptr<IPoolableMaker>&& maker, std::size_t size) :
		m_object_maker(std::move(maker)),
		m_pool_size(size)
	{
		// Create objects
		if (m_object_maker != nullptr)
		{
			for (std::size_t i = 0; i < m_pool_size; ++i)
			{
				m_pool.push_back( m_object_maker->create() );
			}
		}
	}

	// Destructor
	CObjectPool::~CObjectPool()
	{
		// Delete objects
		for (auto object : m_pool)
		{
			delete object;
		}
	}

	// Take an object from the pool
	IPoolable* CObjectPool::acquire()
	{
		// Lock object pool
		std::lock_guard<std::mutex> lock(m_pool_mtx);

		// Check that there are objects in the pool
		if (m_pool.empty() == true)
		{
			throw object_pool_exception(
				"Failed to acquire an object because the object pool is empty."
			);
		}

		// Take the object
		IPoolable* object = m_pool.front();

		// Pop the object out of the pool
		m_pool.pop_front();

		// Give the object to the consumer
		return object;
	}

	// Return an object to the pool
	void CObjectPool::release(IPoolable* object)
	{
		// Check pointer to the object
		if (object == nullptr)
		{
			return;
		}

		// Reset the object state
		object->reset();
		
		{
			// Lock object pool
			std::lock_guard<std::mutex> lock(m_pool_mtx);

			// Push the object to the pool
			m_pool.push_back(object);
		}
	}

	// Get number of objects in the object pool that can be taken
	std::size_t CObjectPool::available() const
	{
		// Objects number
		std::size_t count = 0;

		{
			// Lock object pool
			std::lock_guard<std::mutex> lock(m_pool_mtx);

			// Get the number of available objects
			count = m_pool.size();
		}

		// Give the object number to the consumer
		return count;
	}
}