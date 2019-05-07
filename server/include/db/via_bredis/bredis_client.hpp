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

#include "log_storage.hpp"
#include "subscriptor.hpp"
#include "user_manage.hpp"

#include "logging.hpp"

namespace chatroom
{
	namespace db
	{
		namespace via_bredis
		{
			class main_connection_async_lock
			{
			public:
				main_connection_async_lock()
					:locked(false)
				{}

				void get()
				{
					if (locked)
					{
						throw tinychat::utility::worded_exception("redis_client : main connection already in use");
					}
					locked = true;
				}

				bool try_get()
				{
					if (locked)
					{
						auto &logger = tinychat::logging::logger::instance();
						logger.warning("bredis_client") << "main connection already in use";
						return false;
					}
					locked = true;
					return true;
				}

				void free() noexcept
				{
					locked = false;
				}

			private:
				bool locked;
			};

			class bredis_client
			{
			public:
				typedef typename bredis::Connection<boost::asio::ip::tcp::socket &> BredisConnection;

				bredis_client(boost::asio::io_context &ioc, boost::asio::ip::tcp::endpoint ep)
					:ioc(ioc), 
					socket(ioc, ep.protocol()),
					main_connection(socket)
				{
					socket.connect(ep);

					boost::asio::ip::tcp::socket subscript_socket(ioc, ep.protocol());
					subscript_socket.connect(ep);
					subscriptor_ = std::make_unique<via_bredis::subscriptor>(std::move(subscript_socket));

					user_name_auth_.reload_sync(this->main_connection);
					banlist_.reload_sync(this->main_connection);

					subscriptor_->bind("user_update", [this]() {
						boost::asio::spawn(this->ioc, [this](boost::asio::yield_context yield)
						{
							if (!this->lock.try_get()) return;
							try
							{
								this->user_name_auth_.refresh(this->main_connection, yield);
							}
							catch (const std::exception &e)
							{
								auto &logger = tinychat::logging::logger::instance();
								logger.error("bredis_client") << "while user_update : " << e.what();
							}
							this->lock.free();
						});
					});

					subscriptor_->bind("user_ban", [this]() {
						boost::asio::spawn(this->ioc, [this](boost::asio::yield_context yield)
						{
							if (!this->lock.try_get()) return;
							try
							{
								this->banlist_.refresh(this->main_connection, yield);
							}
							catch (const std::exception &e)
							{
								auto &logger = tinychat::logging::logger::instance();
								logger.error("bredis_client") << "while user_ban : " << e.what();
							}
							this->lock.free();
						});
					});

					subscriptor_->start(ioc, "tinychat");
				}

				template<typename Generator, typename Handle>
				void log_storage_checkin(Generator next, Handle ok_callback)
				{
					boost::asio::spawn(this->ioc, [next, ok_callback, this](boost::asio::yield_context yield)
					{
						if (!this->lock.try_get()) return;
						try
						{
							this->log_storage_.checkin(this->main_connection, next, yield);
						}
						catch (const std::exception &e)
						{
							auto &logger = tinychat::logging::logger::instance();
							logger.warning("bredis_client") << "while log_checkin : " << e.what();
						}
						this->lock.free();
						ok_callback();
					});
				}

				via_bredis::user_manage::user_name_auth_cache &user_name_auth()
				{
					return user_name_auth_;
				}

				via_bredis::user_manage::banlist_cache &banlist()
				{
					return banlist_;
				}

				via_bredis::log_storage &log_storage()
				{
					return log_storage_;
				}

				bool authenticate(const std::string &name, const std::string &auth)
				{
					return (this->user_name_auth_[name] == auth);
				}

				bool is_banned(const std::string &name)
				{
					return this->banlist_[name];
				}

			private:
				boost::asio::io_context &ioc;
				boost::asio::ip::tcp::socket socket;
				BredisConnection main_connection;
				main_connection_async_lock lock;

				std::unique_ptr<via_bredis::subscriptor> subscriptor_;

				via_bredis::log_storage log_storage_;
				via_bredis::user_manage::user_name_auth_cache user_name_auth_;
				via_bredis::user_manage::banlist_cache banlist_;

			};

		};
	};
};
