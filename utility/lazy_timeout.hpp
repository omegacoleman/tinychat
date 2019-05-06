#pragma once

#include <list>
#include <algorithm>
#include <chrono>

#include "boost/asio.hpp"
#include "boost/asio/spawn.hpp"

#include "boost_system_exception.hpp"

namespace tinychat
{
	namespace utility
	{
		// A "lazy timeout" will scan every binded AvailHandle, every setted interval
		// If a binded AvailHandle returned false 2 times in a row, then the 
		// TimeoutHandle packed with it is called.
		// If a binded handle is expired(cannot be called anymore), it shall be 
		// "uninstalled" using the returned uninstall handle.
		class lazy_timeout
		{
		public:
			using AvailHandle = std::function<bool(void)>;
			using TimeoutHandle = std::function<void(void)>;
			using UninstallHandle = std::function<void(void)>;

			lazy_timeout(std::chrono::nanoseconds interval)
				:interval(interval)
			{}

			UninstallHandle bind(AvailHandle avail, TimeoutHandle timeout)
			{
				this->binded.push_front({ avail, timeout });
				auto inserted = this->binded.begin();
				return [inserted, this]()
				{
					inserted->uninstalled = true;
				};
			}

			void scan()
			{
				binded.erase(std::remove_if(binded.begin(), binded.end(), [](binded_t& it) -> bool {
					if (it.uninstalled)
					{
						return true;
					}
					bool avail_now = it.avail_func();
					bool timed_out = !(avail_now || it.last_check);
					if (timed_out)
					{
						it.timeout_func();
					}
					it.last_check = avail_now;
					return false;
				}), binded.end());
			}

			void scan_proc(boost::asio::io_context &ioc, boost::asio::yield_context yield)
			{
				try
				{
					boost::asio::steady_timer timer(ioc);
					while (true)
					{
						boost::system::error_code ec;
						timer.expires_after(this->interval);
						timer.async_wait(yield[ec]); _RT_EC("wait(lazy_scan)", ec);
						try
						{
							this->scan();
						}
						catch (const std::exception &e)
						{
							std::cerr << "lazy_timeout : during timeout scan : " << e.what() << std::endl;
						}
					}
				}
				catch (const std::exception &e)
				{
					std::cerr << "lazy_timeout : timer : " << e.what() << std::endl;
				}
			}

			void start_scan(boost::asio::io_context &ioc)
			{
				boost::asio::spawn(ioc, [&ioc, this](boost::asio::yield_context yield)
				{
					this->scan_proc(ioc, yield);
				});
			}

		private:
			class binded_t
			{
			public:
				binded_t(AvailHandle avail_func, TimeoutHandle timeout_func)
					: avail_func(avail_func),
					timeout_func(timeout_func),
					last_check(true),
					uninstalled(false)
				{}

				AvailHandle avail_func;
				TimeoutHandle timeout_func;
				bool last_check;
				bool uninstalled;
			};
			std::list<binded_t> binded;
			std::chrono::nanoseconds interval;
		};
	}
}

