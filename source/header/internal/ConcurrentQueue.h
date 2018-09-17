//@HIPE_LICENSE@
#pragma once
#include <queue>
#include <atomic>
#pragma warning(push, 0)
#include <boost/thread.hpp>
#pragma warning(pop)

namespace core
{
	namespace queue
	{
		template<typename Data>
		class ConcurrentQueue
		{
		private:
			std::queue<Data> the_queue;
			mutable boost::mutex the_mutex;
			boost::condition_variable the_condition_variable;
			std::atomic<bool> _listerners;
		public:
			ConcurrentQueue() { _listerners = false; }

			void stopListening()
			{
				_listerners = false;
			}

			~ConcurrentQueue()
			{
				_listerners = false;
			};

			void push(Data const& data)
			{
				boost::mutex::scoped_lock lock(the_mutex);
				the_queue.push(data);
				lock.unlock();
				the_condition_variable.notify_one();
			}

			bool empty() const
			{
				boost::mutex::scoped_lock lock(the_mutex);
				return the_queue.empty();
			}

			size_t size() 
			{
				boost::mutex::scoped_lock lock(the_mutex);
				return the_queue.size();
			}

			void clear()
			{
				boost::mutex::scoped_lock lock(the_mutex);
				std::queue<Data> empty;
				std::swap(the_queue, empty);
			}

			bool pop(Data& popped_value)
			{
				return try_pop(popped_value);
			}

			bool try_pop(Data& popped_value)
			{
				boost::mutex::scoped_lock lock(the_mutex);
				if (the_queue.empty())
				{
					return false;
				}

				popped_value = the_queue.front();
				the_queue.pop();
				return true;
			}

			void wait_and_pop(Data& popped_value)
			{
				boost::mutex::scoped_lock lock(the_mutex);
				while (the_queue.empty())
				{
					the_condition_variable.wait(lock);
					
				}

				popped_value = the_queue.front();
				the_queue.pop();
			}

			bool waituntil_and_pop(Data& popped_value)
			{
				boost::mutex::scoped_lock lock(the_mutex);
				while (the_queue.empty())
				{
					the_condition_variable.timed_wait(lock, boost::posix_time::milliseconds(1000));
					if (!hasListener()) 
						return false;
				}

				popped_value = the_queue.front();
				the_queue.pop();
				return true;
			}

			bool trypop_until(Data& popped_value, int ms)
			{
				boost::mutex::scoped_lock lock(the_mutex);
				the_condition_variable.timed_wait(lock, boost::posix_time::milliseconds(ms));
				if (the_queue.empty())
				{
					return false;
				}

				popped_value = the_queue.front();
				the_queue.pop();
				return true;
			}
			
			void readyToListen() { _listerners = true; }

			bool hasListener() { return _listerners; }

		};
	}
}
