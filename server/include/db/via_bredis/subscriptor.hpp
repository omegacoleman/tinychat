#pragma once

#include "boost/asio.hpp"
#include "boost/asio/spawn.hpp"

#include <memory>

#include "impl/subscriptor.ipp"

namespace chatroom
{
	namespace db
	{
		namespace via_bredis
		{

			class subscriptor 
			{
			public:
				typedef typename impl::base_subscriptor<
					boost::asio::ip::tcp::socket,
					boost::asio::streambuf,
					std::function<void()>
				> impl_t;

				// Create a subscriptor with a socket
				subscriptor(boost::asio::ip::tcp::socket && _socket)
					:detail(std::forward<boost::asio::ip::tcp::socket &&>(_socket))
				{}

				// Start the subscriptor on specified channel, runs in a new yield context from ioc
				void start(boost::asio::io_context &ioc, const std::string &channel)
				{
					detail.init_command(channel);
					boost::asio::spawn(
						ioc, [channel, this](boost::asio::yield_context yield)
					{
						this->detail.run_loop(channel, yield);
					});
				}

				// Bind a void func() handler to specified string, will be called if the subscriptor
				// gets a message equal to the str
				void bind(const std::string &on_str, std::function<void()> handler)
				{
					detail.bind(on_str, std::move(handler));
				}

			private:
				impl_t detail;
			};

		};
	};
};

