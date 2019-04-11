#pragma once

#include <memory>

#include "bredis.hpp"

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

				void reload_users_sync()
				{
					this->user_auth_map.clear();

					boost::asio::streambuf rx_buff;
					this->c->write(bredis::single_command_t{ "HGETALL", "users" });
					auto parse_result = this->c->read(rx_buff);
					auto extract = boost::apply_visitor(
						bredis::extractor<bredis::to_iterator<boost::asio::streambuf>::iterator_t>(),
						parse_result.result);
					rx_buff.consume(parse_result.consumed);
					auto &reply_arr = boost::get<bredis::extracts::array_holder_t>(extract);
					for (auto it = reply_arr.elements.begin(); it != reply_arr.elements.end();)
					{
						auto &user_str = boost::get<bredis::extracts::string_t>(*it);
						++it;
						auto &auth_str = boost::get<bredis::extracts::string_t>(*it);
						++it;
						this->user_auth_map.insert(std::make_pair(user_str.str, auth_str.str));
						std::cout << "loading user : " << user_str.str << " $$ " << auth_str.str << std::endl;
					}
				}

				// TODO : auto call this upon user update (called via redis subscription)
				template <typename user_update_handle = std::function<void(const std::string, const std::string) > >
				void refresh_users(user_update_handle user_update, boost::asio::yield_context yield)
				{
					boost::asio::streambuf tx_buff, rx_buff;
					boost::system::error_code ec;
					std::size_t consumed = this->c->async_write(tx_buff, bredis::single_command_t{ "HGETALL", "users" }, yield[ec]);
					if (ec)
					{
						std::cerr << "bredis_client.hpp : error refreshing user list " << ec.message() << std::endl;
					}
					tx_buff.consume(consumed);
					auto parse_result = this->c->async_read(rx_buff, yield[ec]);
					auto extract = boost::apply_visitor(
						bredis::extractor<bredis::to_iterator<boost::asio::streambuf>::iterator_t>(),
						parse_result.result);
					rx_buff.consume(parse_result.consumed);
					auto &reply_arr = boost::get<bredis::extracts::array_holder_t>(extract);
					for (auto it = reply_arr.elements.begin(); it != reply_arr.elements.end();)
					{
						auto &user_str = boost::get<bredis::extracts::string_t>(*it);
						++it;
						auto &auth_str = boost::get<bredis::extracts::string_t>(*it);
						++it;
						if (! this->user_auth_map.count(user_str.str))
						{
							this->user_auth_map.insert(std::make_pair(user_str.str, auth_str.str));
							std::cout << "adding user : " << user_str.str << " $$ " << auth_str.str << std::endl;
							user_update(user_str.str, auth_str.str);
						}
					}
				}

				using user_auth_pair_iterator = std::map<std::string, std::string>::const_iterator;

				std::pair<user_auth_pair_iterator, user_auth_pair_iterator> users()
				{
					return std::make_pair(this->user_auth_map.cbegin(), this->user_auth_map.cend());
				}

				void checkin_log_proc(chatroom::LogIterator it, chatroom::LogIterator end, std::function<void()> done_callback, boost::asio::yield_context yield)
				{
					boost::asio::streambuf tx_buff, rx_buff;
					boost::system::error_code ec;

					auto eq_OK = bredis::marker_helpers::equality<result_iterator>("OK");
					auto eq_one = bredis::marker_helpers::equality<result_iterator>("1");

					std::cout << "writing to db : ";

					bool all_ok = true;

					for (; it != end; ++it)
					{
						bredis::command_container_t container;

						container.push_back(bredis::single_command_t{ "set", it->id, it->text });

						std::string key_sender = it->id + ":sender";
						container.push_back(bredis::single_command_t{ "set", key_sender, it->sender });

						std::string key_unix_time = it->id + ":unix_time";
						std::string s_unix_time = std::to_string(it->unix_time);
						container.push_back(bredis::single_command_t{ "set", key_unix_time, s_unix_time });

						std::vector<std::string> log_by_unix_time_cmd;
						log_by_unix_time_cmd.push_back("zadd");
						log_by_unix_time_cmd.push_back("log_by_unix_time");
						log_by_unix_time_cmd.push_back(std::to_string(it->unix_time));
						log_by_unix_time_cmd.push_back(it->id);
						container.push_back(bredis::single_command_t(log_by_unix_time_cmd.begin(), log_by_unix_time_cmd.end()));

						std::size_t consumed = this->c->async_write(tx_buff, container, yield[ec]);
						if (ec)
						{
							std::cerr << "bredis_client.hpp: checkin_log_proc : writing to redis server FAILED, error : " << ec.message() << std::endl;
							return;
						}
						tx_buff.consume(consumed);

						auto parse_result = this->c->async_read(rx_buff, yield[ec], container.size());
						if (ec)
						{
							std::cerr << "bredis_client.hpp: checkin_log_proc : reading from redis server FAILED, error : " << ec.message() << std::endl;
							return;
						}
						auto &replies = boost::get<bredis::markers::array_holder_t<result_iterator> >(parse_result.result);

						bool this_ok = true;
						this_ok = this_ok && boost::apply_visitor(eq_OK, replies.elements[0]);
						this_ok = this_ok && boost::apply_visitor(eq_OK, replies.elements[1]);
						this_ok = this_ok && boost::apply_visitor(eq_OK, replies.elements[2]);
						this_ok = this_ok && boost::apply_visitor(eq_one, replies.elements[3]);
						if (this_ok)
						{
							std::cout << "+";
						}
						else {
							std::cout << "-";
						}
						all_ok = all_ok && this_ok;
						rx_buff.consume(parse_result.consumed);
					}
					std::cout << std::endl;
					if (all_ok)
					{
						done_callback();
					}
					else {
						std::cerr << "bredis_client.hpp : some items failed to write to db" << std::endl;
					}
					
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
				std::map<std::string, std::string> user_auth_map;
			};

		};
	};
};