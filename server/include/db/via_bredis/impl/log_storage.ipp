#pragma once

#include <memory>
#include <map>
#include <functional>

#include "boost/asio.hpp"
#include "boost/asio/spawn.hpp"

#include "bredis.hpp"

#include "chatroom.hpp"

#include "utility/boost_system_exception.hpp"
#include "utility/worded_exception.hpp"

#include "utility/logging.hpp"

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
					typename Buffer
				>
				class base_log_storage
				{
				public:
					typedef typename bredis::Connection<Socket &> BredisConnection;
					typedef typename bredis::to_iterator<Buffer>::iterator_t ResultIterator;
					typedef typename bredis::extractor<ResultIterator> Extractor;

					template <typename Generator>
					bool checkin(BredisConnection &conn, Generator next, boost::asio::yield_context yield)
					{
						boost::system::error_code ec;

						Buffer tx_buff, rx_buff;
						auto &logger = tinychat::logging::logger::instance();
						auto writing_r_collect = logger.info("log_storage");
						writing_r_collect << " writing to db : ";

						bool all_ok = true;

						for (auto it = next(); it != nullptr; it = next())
						{
							bredis::command_container_t container;

							container.push_back(bredis::single_command_t{ "set", it->id, it->text });

							std::string key_sender = it->id + ":sender";
							container.push_back(bredis::single_command_t{ "set", key_sender, it->sender });

							std::string key_unix_time = it->id + ":unix_time";
							std::string s_unix_time = std::to_string(it->unix_time);
							container.push_back(bredis::single_command_t{ "set", key_unix_time, s_unix_time });

							std::vector<std::string> log_by_unix_time_cmd
							{
								"zadd",
								it->room_id + ":" + "log_by_unix_time",
								std::to_string(it->unix_time),
								it->id
							};
							container.push_back(bredis::single_command_t(log_by_unix_time_cmd.begin(), log_by_unix_time_cmd.end()));

							std::size_t consumed = conn.async_write(tx_buff, container, yield[ec]); _RT_EC("write(checkin commands)", ec);
							tx_buff.consume(consumed);

							auto parse_result = conn.async_read(rx_buff, yield[ec], container.size()); _RT_EC("read(checkin commands)", ec);
							auto &replies = boost::get<bredis::markers::array_holder_t<ResultIterator> >(parse_result.result);

							bool this_ok = true;
							this_ok = this_ok && boost::apply_visitor(eq_OK, replies.elements[0]); // set
							this_ok = this_ok && boost::apply_visitor(eq_OK, replies.elements[1]); // set :sender
							this_ok = this_ok && boost::apply_visitor(eq_OK, replies.elements[2]); // set :unix_time
							this_ok = this_ok && boost::apply_visitor(eq_one, replies.elements[3]); // zadd log_by_unix_time
							rx_buff.consume(parse_result.consumed);

							if (this_ok)
							{
								writing_r_collect << "+";
							}
							else {
								writing_r_collect << "-";
							}

							all_ok = all_ok && this_ok;
						}

						if (all_ok)
						{
							return true;
						}
						else {
							return false;
						}
					}

					inline const static auto eq_OK = bredis::marker_helpers::equality<ResultIterator>("OK");
					inline const static auto eq_one = bredis::marker_helpers::equality<ResultIterator>("1");
				};

			};
		};
	};
};
