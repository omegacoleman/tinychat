#pragma once

#include <memory>
#include <map>
#include <unordered_set>
#include <functional>

#include "boost/asio.hpp"
#include "boost/asio/spawn.hpp"

#include "bredis.hpp"

#include "boost_system_exception.hpp"
#include "worded_exception.hpp"

#include "logging.hpp"

namespace chatroom
{
	namespace db
	{
		namespace via_bredis
		{
			namespace user_manage
			{
				namespace impl
				{

					template <
						typename Socket, 
						typename Buffer
					>
					class base_user_name_auth_cache
					{
					public:
						typedef typename bredis::Connection<Socket &> BredisConnection;
						typedef typename bredis::to_iterator<Buffer>::iterator_t ResultIterator;
						typedef typename bredis::extractor<ResultIterator> Extractor;

						std::function<void(const std::string &)> on_add_user;
						std::function<void(const std::string &, const std::string &)> on_auth_change;

						void reload_sync(BredisConnection &conn)
						{
							auto &logger = tinychat::logging::logger::instance();
							this->cache.clear();

							Buffer rx_buff;
							conn.write(bredis::single_command_t{ "HGETALL", "users" });

							auto parse_result = conn.read(rx_buff);
							auto extract = boost::apply_visitor(Extractor(), parse_result.result);
							rx_buff.consume(parse_result.consumed);

							auto &reply_arr = boost::get<bredis::extracts::array_holder_t>(extract);
							for (auto it = reply_arr.elements.begin(); it != reply_arr.elements.end();)
							{
								auto &user_str = boost::get<bredis::extracts::string_t>(*it);
								++it;
								auto &auth_str = boost::get<bredis::extracts::string_t>(*it);
								++it;
								this->cache.insert(std::make_pair(user_str.str, auth_str.str));
								logger.info("user_manage") << "loading user : " << user_str.str;
							}
						}

						void refresh(BredisConnection &conn, boost::asio::yield_context yield)
						{
							auto &logger = tinychat::logging::logger::instance();
							boost::system::error_code ec;

							logger.info("user_manage") << "refreshing user list..";

							Buffer tx_buff, rx_buff;
							auto consumed = conn.async_write(
								tx_buff, bredis::single_command_t{ "HGETALL", "users" }, yield[ec]
							); _RT_EC("write(HGETALL users)", ec);
							tx_buff.consume(consumed);

							auto parse_result = conn.async_read(rx_buff, yield[ec]); _RT_EC("read(HGETALL users)", ec);
							auto extract = boost::apply_visitor(Extractor(), parse_result.result);
							rx_buff.consume(parse_result.consumed);

							auto &reply_arr = boost::get<bredis::extracts::array_holder_t>(extract);
							for (auto it = reply_arr.elements.begin(); it != reply_arr.elements.end();)
							{
								auto &user_str = boost::get<bredis::extracts::string_t>(*it);
								++it;
								auto &auth_str = boost::get<bredis::extracts::string_t>(*it);
								++it;
								if (!this->cache.count(user_str.str))
								{
									this->cache.insert(std::make_pair(user_str.str, auth_str.str));
									logger.info("user_manage") << "adding user : " << user_str.str;
									if (on_add_user) on_add_user(user_str.str);
								}
								else {
									if (this->cache[user_str.str] != auth_str.str)
									{
										this->cache[user_str.str] = auth_str.str;
										logger.info("user_manage") << "changed user auth str : " << user_str.str;
										if (on_auth_change) on_auth_change(user_str.str, auth_str.str);
									}
								}
							}
						}

						const std::string &get_auth(const std::string &name)
						{
							return cache.at(name);
						}

						template <typename Callback>
						void for_each_name(Callback cb)
						{
							std::for_each(cache.begin(), cache.end(), [cb](std::map<std::string, std::string>::const_reference pair)
							{
								cb(pair.first);
							});
						}

						std::map<std::string, std::string> cache;
					};



					template <
						typename Socket,
						typename Buffer
					>
					class base_banlist_cache
					{
					public:
						typedef typename bredis::Connection<Socket &> BredisConnection;
						typedef typename bredis::to_iterator<Buffer>::iterator_t ResultIterator;
						typedef typename bredis::extractor<ResultIterator> Extractor;

						std::function<void(const std::string &)> on_ban;
						std::function<void(const std::string &)> on_unban;

						void reload_sync(BredisConnection &conn)
						{
							auto &logger = tinychat::logging::logger::instance();
							this->cache.clear();

							Buffer rx_buff;
							conn.write(bredis::single_command_t{ "SMEMBERS", "user:banned" });

							auto parse_result = conn.read(rx_buff);
							auto extract = boost::apply_visitor(Extractor(), parse_result.result);
							rx_buff.consume(parse_result.consumed);

							auto &reply_arr = boost::get<bredis::extracts::array_holder_t>(extract);
							for (auto it = reply_arr.elements.begin(); it != reply_arr.elements.end(); ++it)
							{
								auto &user_str = boost::get<bredis::extracts::string_t>(*it);
								this->cache.insert(user_str.str);
								logger.info("user_manage") << "loading banned user : " << user_str.str;
							}
						}

						void refresh(BredisConnection &conn, boost::asio::yield_context yield)
						{
							auto &logger = tinychat::logging::logger::instance();
							boost::system::error_code ec;

							logger.info("user_manage") << "refreshing user list..";

							Buffer tx_buff, rx_buff;
							auto consumed = conn.async_write(
								tx_buff, bredis::single_command_t{ "SMEMBERS", "user:banned" }, yield[ec]
							); _RT_EC("write(HGETALL users)", ec);
							tx_buff.consume(consumed);

							auto parse_result = conn.async_read(rx_buff, yield[ec]); _RT_EC("read(SMEMBERS user:banned)", ec);
							auto extract = boost::apply_visitor(Extractor(), parse_result.result);
							rx_buff.consume(parse_result.consumed);

							auto &reply_arr = boost::get<bredis::extracts::array_holder_t>(extract);
							std::unordered_set<std::string> new_banned;
							for (auto it = reply_arr.elements.begin(); it != reply_arr.elements.end(); ++it)
							{
								auto &user_str = boost::get<bredis::extracts::string_t>(*it);
								new_banned.insert(user_str.str);
								if (cache.count(user_str.str) == 0)
								{
									this->cache.insert(user_str.str);
									logger.info("user_manage") << "banned user : " << user_str.str;
									if (on_ban) on_ban(user_str.str);
								}
							}

							// not very pretty, will fix later
							for (auto it = cache.begin(); it != cache.end();)
							{
								if (!new_banned.count(*it))
								{
									logger.info("user_manage") << "unbanned user : " << *it;
									if (on_unban) on_unban(*it);
									it = cache.erase(it);
								}
								else {
									it++;
								}
							}
						}

						bool is_banned(const std::string &name)
						{
							return (cache.count(name) == 1);
						}

						std::unordered_set<std::string> cache;
					};


				};
			};
		};
	};
};
