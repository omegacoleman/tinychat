#pragma once

#include <memory>

#include "bredis/MarkerHelpers.hpp"
#include "bredis/Connection.hpp"

#include "boost/asio.hpp"
#include "boost/asio/spawn.hpp"

#include "chatroom.hpp"

namespace chatroom
{
	namespace db
	{
		namespace via_bredis
		{
			using result_iterator = typename bredis::to_iterator<boost::asio::streambuf>::iterator_t;

			class connection
			{
			public:
				connection(boost::asio::io_context &ioc, const std::string host, const unsigned short port)
					:ioc(ioc)
				{
					boost::asio::ip::tcp::endpoint end(
						boost::asio::ip::address::from_string(host), port);
					this->s = std::make_unique<boost::asio::ip::tcp::socket>(ioc, end.protocol());
					this->s->connect(end);

					this->c = std::make_unique<bredis::Connection<boost::asio::ip::tcp::socket &> >(*this->s);
				}

				void checkin_log_proc(chatroom::LogIterator it, chatroom::LogIterator end, std::function<void()> done_callback, boost::asio::yield_context yield)
				{
					boost::asio::streambuf tx_buff, rx_buff;
					boost::system::error_code ec;
					size_t count = 0;
					for (; it != end; ++it)
					{
						std::size_t consumed = this->c->async_write(tx_buff, bredis::single_command_t{ "set", it->id, it->text }, yield[ec]);
						if (ec)
						{
							std::cerr << "bredis.hpp: checkin_log_proc : writing to redis server FAILED, error : " << ec.message() << std::endl;
							return;
						}
						tx_buff.consume(consumed);
						count++;
					}
					auto parse_result = this->c->async_read(rx_buff, yield[ec], count);
					if (ec)
					{
						std::cerr << "bredis.hpp: checkin_log_proc : reading from redis server FAILED, error : " << ec.message() << std::endl;
						return;
					}
					auto eq_OK = bredis::marker_helpers::equality<result_iterator>("OK");
					auto &replies = boost::get<bredis::markers::array_holder_t<result_iterator> >(parse_result.result);
					std::cout << "writing done, result ";
					for (auto itt = replies.elements.begin(); itt != replies.elements.end(); ++itt)
					{
						if (boost::apply_visitor(eq_OK, *itt))
						{
							std::cout << "+";
						}
						else {
							std::cout << "-";
						}
					}
					rx_buff.consume(parse_result.consumed);
					done_callback();
				}

				void checkin_log(chatroom::LogIterator it, chatroom::LogIterator end, std::function<void()> done_callback)
				{
					boost::asio::spawn(this->ioc,
						std::bind(
							&connection::checkin_log_proc,
							this,
							it, end, done_callback,
							std::placeholders::_1));
				}
			private:
				boost::asio::io_context &ioc;
				std::unique_ptr<boost::asio::ip::tcp::socket> s;
				std::unique_ptr<bredis::Connection<boost::asio::ip::tcp::socket &> > c;
			};

		};
	};
};