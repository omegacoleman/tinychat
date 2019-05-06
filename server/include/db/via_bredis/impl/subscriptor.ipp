#pragma once

#include "boost/asio.hpp"
#include "boost/asio/spawn.hpp"

#include <memory>
#include <map>

#include "bredis.hpp"

#include "boost_system_exception.hpp"
#include "worded_exception.hpp"

namespace chatroom
{
	namespace db
	{
		namespace via_bredis
		{

			namespace impl
			{

				template <
					typename Socket,
					typename Buffer,
					typename CallbackHandler
				>
					class base_subscriptor
				{
				public:
					typedef typename bredis::Connection<Socket &> BredisConnection;

					typedef typename bredis::to_iterator<Buffer>::iterator_t ResultIterator;
					typedef typename bredis::parse_result_mapper_t<ResultIterator, bredis::parsing_policy::keep_result> ParseResult;

					typedef typename bredis::marker_helpers::check_subscription<ResultIterator> InitialResultChecker;

					typedef typename bredis::extractor<ResultIterator> Extractor;

					base_subscriptor(Socket && _socket)
						:socket(std::move(_socket)),
						redis_connection(socket),
						subcription_callbacks()
					{}

					void init_command(const std::string &channel)
					{
						boost::system::error_code ec;
						Buffer rx_buff;
						auto command = bredis::single_command_t{ "subscribe", channel };
						this->redis_connection.write(command);
						std::cout << "bredis_client.hpp : starting subscription.." << std::endl;

						auto initial_result = this->redis_connection.read(rx_buff);
						if (!boost::apply_visitor(InitialResultChecker(command), initial_result.result))
						{
							throw tinychat::utility::worded_exception("subscription failed : wrong answer from server");
						}
						rx_buff.consume(initial_result.consumed);
						std::cout << "bredis_client.hpp : subscription started" << std::endl;
					}

					void run_loop(const std::string &channel, boost::asio::yield_context yield) noexcept
					{
						boost::system::error_code ec;

						Buffer rx_buff;

						while (true)
						{
							try
							{
								ParseResult parse_result = this->redis_connection.async_read(rx_buff, yield[ec]); _RT_EC("read(subscribe)", ec);
									std::cout << "bredis_client.hpp : subscription got something" << std::endl;

								auto extract = boost::apply_visitor(Extractor(), parse_result.result);
								rx_buff.consume(parse_result.consumed);
								std::string target = extract_message(channel, extract);
								try
								{
									subcription_callbacks[target]();
								}
								catch (const std::exception &e)
								{
									std::cerr << "bredis_client.hpp : subscription mission failed -- " << e.what() << std::endl;
								}
							}
							catch (const std::exception &e)
							{
								std::cerr << "bredis_client.hpp : subscription error -- " << e.what() << std::endl;
							}
						}
					}

					void bind(const std::string &on_str, CallbackHandler handler)
					{
						subcription_callbacks[on_str] = handler;
					}

					template<typename Extracted>
					std::string extract_message(const std::string &channel, const Extracted& extract)
					{
						auto &message_array = boost::get<bredis::extracts::array_holder_t>(extract);
						if (message_array.elements.size() != 3)
						{
							throw tinychat::utility::worded_exception("subscription message : size() != 3");
						}

						auto &str_message = boost::get<bredis::extracts::string_t>(message_array.elements[0]);
						auto &str_channel = boost::get<bredis::extracts::string_t>(message_array.elements[1]);
						if (str_message.str != "message" || str_channel.str != channel)
						{
							throw tinychat::utility::worded_exception("subscription message : format error");
						}

						return std::move(boost::get<bredis::extracts::string_t>(message_array.elements[2]).str);
					}

					Socket socket;
					BredisConnection redis_connection;
					std::map<std::string, CallbackHandler> subcription_callbacks;
				};

			};
		};
	};
};
