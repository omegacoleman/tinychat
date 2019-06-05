#pragma once

#include <memory>
#include <functional>

#include "boost/asio.hpp"
#include "boost/asio/spawn.hpp"

#include "utility/boost_system_exception.hpp"
#include "utility/worded_exception.hpp"

#include "impl/user_manage.ipp"

namespace chatroom
{
	namespace db
	{
		namespace via_bredis
		{
			namespace user_manage
			{

				class user_name_auth_cache
				{
				public:
					typedef typename impl::base_user_name_auth_cache<
						boost::asio::ip::tcp::socket,
						boost::asio::streambuf
					> impl_t;
					// Completly reload the data from DB
					void reload_sync(impl_t::BredisConnection &conn)
					{
						detail.reload_sync(conn);
					}

					// Fetch data & update the cache, emit events when needed
					void refresh(impl_t::BredisConnection &conn, boost::asio::yield_context yield)
					{
						detail.refresh(conn, yield);
					}

					// Return auth from name
					const std::string &get_auth(const std::string &name)
					{
						return detail.get_auth(name);
					}

					// Return auth from name
					const std::string &operator[](const std::string &name)
					{
						return detail.get_auth(name);
					}

					template <typename Callback>
					void for_each_name(Callback cb)
					{
						detail.for_each_name(cb);
					}

					void set_on_add_user(std::function<void(const std::string &)> on_add_user)
					{
						this->detail.on_add_user = on_add_user;
					}

					void set_on_auth_change(std::function<void(const std::string &, const std::string &)> on_auth_change)
					{
						this->detail.on_auth_change = on_auth_change;
					}

				private:
					impl_t detail;
				};

				class banlist_cache
				{
				public:
					typedef typename impl::base_banlist_cache<
						boost::asio::ip::tcp::socket,
						boost::asio::streambuf
					> impl_t;

					// Completly reload the data from DB
					void reload_sync(impl_t::BredisConnection &conn)
					{
						detail.reload_sync(conn);
					}

					// Fetch data & update the cache, emit events when needed
					void refresh(impl_t::BredisConnection &conn, boost::asio::yield_context yield)
					{
						detail.refresh(conn, yield);
					}

					// Check if name is in the banlist
					bool is_banned(const std::string &name)
					{
						return detail.is_banned(name);
					}

					// Check if name is in the banlist
					bool operator[](const std::string &name)
					{
						return detail.is_banned(name);
					}

					void set_on_ban(std::function<void(const std::string &)> on_ban)
					{
						this->detail.on_ban = on_ban;
					}

					void set_on_unban(std::function<void(const std::string &)> on_unban)
					{
						this->detail.on_unban = on_unban;
					}
				private:
					impl_t detail;
				};

			};
		};
	};
};
