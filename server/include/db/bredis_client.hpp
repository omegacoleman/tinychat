#pragma once

#include <memory>
#include <map>
#include <functional>

#include "boost/asio.hpp"
#include "boost/asio/spawn.hpp"

#include "bredis.hpp"

#include "chatroom.hpp"

#include "boost_system_exception.hpp"
#include "worded_exception.hpp"

namespace chatroom
{
	namespace db
	{
		namespace via_bredis
		{
			using result_iterator = typename bredis::to_iterator<boost::asio::streambuf>::iterator_t;
			using user_update_handle = std::function<void(const std::string &, const std::string &) >;
			using subscription_handler = std::function<void(boost::asio::yield_context yield)>;
			using user_ban_handler = std::function<void(const std::string &)>;

			class connection
			{
			public:
				connection(boost::asio::io_context &ioc, const std::string host, const unsigned short port)
					:ioc(ioc), occupied(false)
				{
					boost::asio::ip::tcp::endpoint end(
						boost::asio::ip::address::from_string(host), port);
					this->s = std::make_unique<boost::asio::ip::tcp::socket>(ioc, end.protocol());
					this->s->connect(end);
					this->c = std::make_unique<bredis::Connection<boost::asio::ip::tcp::socket &> >(*this->s);

					this->s_subscription = std::make_unique<boost::asio::ip::tcp::socket>(ioc, end.protocol());
					this->s_subscription->connect(end);
					this->c_subscription = std::make_unique<bredis::Connection<boost::asio::ip::tcp::socket &> >(*this->s_subscription);

					boost::asio::spawn(ioc, [this](boost::asio::yield_context yield) mutable
					{
						try
						{
							this->subscription_proc(yield);
						}
						catch (const std::exception &e)
						{
							std::cerr << "bredis_client.hpp : subscription failed : " << e.what() << ", auto-updating features may fail."
								<< std::endl;
						}
					});
				}

				// this thing is good, could be extracted as an util class someday.
				void subscription_proc(boost::asio::yield_context yield)
				{

					using parse_result_t = bredis::parse_result_mapper_t<result_iterator, bredis::parsing_policy::keep_result>;
					using read_callback_t = std::function<void(const boost::system::error_code &error_code, parse_result_t &&r)>;

					boost::asio::streambuf rx_buff;
					boost::system::error_code ec;
					auto command = bredis::single_command_t{ "subscribe", "tinychat" };
					this->c_subscription->write(command);
					std::cout << "bredis_client.hpp : starting subscription.." << std::endl;
					auto initial_parse_result = this->c_subscription->read(rx_buff);
					if (! boost::apply_visitor(
						bredis::marker_helpers::check_subscription<result_iterator>(
							command), initial_parse_result.result))
					{
						throw tinychat::utility::worded_exception("subscription failed");
					}
					rx_buff.consume(initial_parse_result.consumed);
					std::cout << "bredis_client.hpp : subscription started" << std::endl;
					while (true)
					{
						parse_result_t parse_result = this->c_subscription->async_read(rx_buff, yield[ec]);
						if (ec)
						{
							throw tinychat::utility::boost_system_ec_exception(ec);
						}
						std::cout << "bredis_client.hpp : subscription got something" << std::endl;
						auto extract = boost::apply_visitor(bredis::extractor<result_iterator>(), parse_result.result);
						rx_buff.consume(parse_result.consumed);
						auto &reply_arr = boost::get<bredis::extracts::array_holder_t>(extract);
						if (reply_arr.elements.size() != 3)
							continue;
						auto &str_message = boost::get<bredis::extracts::string_t>(reply_arr.elements[0]);
						auto &str_tinychat = boost::get<bredis::extracts::string_t>(reply_arr.elements[1]);
						if (str_message.str != "message" || str_tinychat.str != "tinychat")
							continue;
						auto &str_target = boost::get<bredis::extracts::string_t>(reply_arr.elements[2]);
						std::for_each(subcription_callbacks.begin(), subcription_callbacks.end(), [&](auto &it)
						{
							if (it.first == str_target.str)
							{
								try
								{
									it.second(yield);
								}
								catch (const std::exception &e)
								{
									std::cerr << "bredis_client.hpp : subscription mission failed -- " << e.what() << std::endl;
								}

							}
						});
					}
				}



				void subscription_bind(const std::string &on_str, subscription_handler handler)
				{
					subcription_callbacks[on_str] = handler;
				}

				void auto_refresh_users(user_update_handle user_update)
				{
					subscription_bind(
						"user_update",
						[user_update, this](boost::asio::yield_context yield)
					{
						refresh_users(user_update, yield);
					});
				}

				void auto_refresh_banned_users(user_ban_handler user_ban)
				{
					subscription_bind("user_ban", std::bind(&connection::ban_users, this, user_ban, std::placeholders::_1));
				}

				void ban_users(user_ban_handler user_ban, boost::asio::yield_context yield)
				{
					boost::asio::streambuf tx_buff, rx_buff;
					auto consumed = this->c->async_write(tx_buff, bredis::single_command_t{ "SMEMBERS", "user:banned" }, yield);
					tx_buff.consume(consumed);
					auto parse_result = this->c->async_read(rx_buff, yield);
					auto extract = boost::apply_visitor(bredis::extractor<result_iterator>(), parse_result.result);
					rx_buff.consume(parse_result.consumed);
					auto &reply_arr = boost::get<bredis::extracts::array_holder_t>(extract);

					std::for_each(reply_arr.elements.begin(), reply_arr.elements.end(),
						[this, &user_ban](auto &it)
					{
						auto &user = boost::get<bredis::extracts::string_t>(it);
						user_ban(user.str);
					});
				}

				void refresh_users(user_update_handle user_update, boost::asio::yield_context yield)
				{
					if (occupied) throw tinychat::utility::worded_exception("already occupied");
					occupied = true;
					std::cout << "bredis_client.hpp : refreshing user list.." << std::endl;
					boost::asio::streambuf tx_buff, rx_buff;
					boost::system::error_code ec;
					auto consumed = this->c->async_write(tx_buff, bredis::single_command_t{ "HGETALL", "users" }, yield[ec]);
					if (ec)
					{
						throw tinychat::utility::boost_system_ec_exception(ec);
					}
					tx_buff.consume(consumed);
					auto parse_result = this->c->async_read(rx_buff, yield[ec]);
					if (ec)
					{
						throw tinychat::utility::boost_system_ec_exception(ec);
					}
					auto extract = boost::apply_visitor(
						bredis::extractor<result_iterator>(),
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
							std::cout << "bredis_client.hpp : adding user : " << user_str.str << std::endl;
							user_update(user_str.str, auth_str.str);
						}
					}
					occupied = false;
				}

				using user_auth_pair_iterator = std::map<std::string, std::string>::const_iterator;

				std::map<std::string, std::string> fetch_users()
				{
					boost::asio::streambuf rx_buff;
					this->c->write(bredis::single_command_t{ "HGETALL", "users" });
					auto parse_result = this->c->read(rx_buff);
					auto extract = boost::apply_visitor(
						bredis::extractor<result_iterator>(),
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
						std::cout << "bredis_client.hpp : loading user : " << user_str.str << std::endl;
					}
					return user_auth_map;
				}

				std::unordered_set<std::string> fetch_banned_users()
				{
					boost::asio::streambuf rx_buff;
					this->c->write(bredis::single_command_t{ "SMEMBERS", "user:banned" });
					auto parse_result = this->c->read(rx_buff);
					auto extract = boost::apply_visitor(bredis::extractor<result_iterator>(), parse_result.result);
					rx_buff.consume(parse_result.consumed);
					auto &reply_arr = boost::get<bredis::extracts::array_holder_t>(extract);
					std::unordered_set<std::string> banned_users;
					std::for_each(reply_arr.elements.begin(), reply_arr.elements.end(),
						[&banned_users](auto &it)
					{
						auto banned_user = boost::get<bredis::extracts::string_t>(it).str;
						std::cout << "bredis_client.hpp : loading banned user : " << banned_user << std::endl;
						banned_users.emplace(std::move(banned_user));
					});
					return banned_users;
				}

				void checkin_log_proc(chatroom::ChatLog::LogIterator it, chatroom::ChatLog::LogIterator end, std::function<void()> done_callback, boost::asio::yield_context yield)
				{
					if (occupied) throw tinychat::utility::worded_exception("already occupied");
					occupied = true;
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

						std::vector<std::string> log_by_unix_time_cmd
						{
							"zadd",
							"log_by_unix_time",
							std::to_string(it->unix_time),
							it->id
						};

						container.push_back(bredis::single_command_t(log_by_unix_time_cmd.begin(), log_by_unix_time_cmd.end()));

						std::size_t consumed = this->c->async_write(tx_buff, container, yield[ec]);
						if (ec)
						{
							throw tinychat::utility::boost_system_ec_exception(ec);
						}
						tx_buff.consume(consumed);

						auto parse_result = this->c->async_read(rx_buff, yield[ec], container.size());
						if (ec)
						{
							throw tinychat::utility::boost_system_ec_exception(ec);
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
					occupied = false;
				}

				void checkin_log(chatroom::ChatLog::LogIterator it, chatroom::ChatLog::LogIterator end, std::function<void()> done_callback)
				{
					boost::asio::spawn(this->ioc, [it, end, done_callback, this](boost::asio::yield_context yield)
						mutable {
						try
						{
							this->checkin_log_proc(it, end, done_callback, yield);
						}
						catch (const std::exception &e)
						{
							std::cerr << "bredis_client.hpp : checkin_log failed, reason : " << e.what() << std::endl;
						}
					});
				}

				bool occupied;

			private:
				boost::asio::io_context &ioc;

				std::unique_ptr<boost::asio::ip::tcp::socket> s;
				std::unique_ptr<bredis::Connection<boost::asio::ip::tcp::socket &> > c;

				std::unique_ptr<boost::asio::ip::tcp::socket> s_subscription;
				std::unique_ptr<bredis::Connection<boost::asio::ip::tcp::socket &> > c_subscription;
				std::map<std::string, std::function<void(boost::asio::yield_context yield)> > subcription_callbacks;

				std::map<std::string, std::string> user_auth_map;
			};

		};
	};
};
