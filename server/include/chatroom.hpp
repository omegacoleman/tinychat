#pragma once

#include <map>
#include <ctime>
#include <utility>
#include <memory>
#include <deque>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <optional>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <unordered_set>

namespace chatroom
{
	#define CHAT_EXCEPTION_STRING(info) (("error in chatroom.hpp : " info))
	class ChatException : public std::exception
	{
	public:
		const char* what() const noexcept override
		{
			return CHAT_EXCEPTION_STRING("unknown");
		}
	};

	#define CHAT_EXCEPTION_CLASS(classname, info) class classname : public ChatException \
	{ \
	public: \
		virtual const char* what() const noexcept \
		{ \
			return CHAT_EXCEPTION_STRING(info); \
		} \
	}

	class Message : public std::enable_shared_from_this<Message>
	{
	public:

		Message(const std::string &room_id, const std::string &sender, const std::string &text, uint64_t unix_time = 0)
			: sender(sender), text(text),
			unix_time(unix_time ? unix_time : std::time(NULL)),
			id_uuid(boost::uuids::random_generator()()), 
			room_id(room_id), 
			id(room_id + ":" + boost::uuids::to_string(this->id_uuid))
		{
		}

		bool operator==(const Message &&m) const &&
		{
			return (this->id_uuid == m.id_uuid);
		}

		bool operator!=(const Message &&m) const &&
		{
			return (this->id_uuid != m.id_uuid);
		}

		std::string sender;
		std::string text;
		uint64_t unix_time;
		boost::uuids::uuid id_uuid;
		std::string id;
		std::string room_id;
	};

	CHAT_EXCEPTION_CLASS(CheckInTooMuchException, "ChatLog : checkin() more log than could");
	CHAT_EXCEPTION_CLASS(ReleaseTooMuchException, "ChatLog : release() more log than could");
	CHAT_EXCEPTION_CLASS(SizeUndesiredException, 
		"ChatLog : checkin_bundle_size shall be under size_limit, and "
		"log_revise_size shall be under floor(size_limit / 2)");

	class ChatLog
	{
	public:
		ChatLog(size_t size_limit, size_t checkin_bundle_size, 
			size_t log_revise_size)
		: size_limit(size_limit),
		checkin_bundle_size(checkin_bundle_size), 
		log_revise_size(log_revise_size), 
		logs(),
		checkin_i(0) {
			if(checkin_bundle_size >= size_limit)
			{
				throw SizeUndesiredException();
			}
			if(log_revise_size >= (size_limit / 2))
			{
				throw SizeUndesiredException();
			}
		}

		using NextMessageHandler = std::function<Message *()>;
		using DoneHander = std::function<void()>;
		using CheckinHandler = std::function<void(NextMessageHandler, DoneHander)>;

		template <typename message_iterator>
		void load_from_db(message_iterator it, message_iterator end)
		{
			const size_t to_load_max = this->size_limit / 2;
			for (; it != end; ++it)
			{
				this->logs.push_front(*it);
				this->checkin_i++;
				if (this->logs.size() >= to_load_max)
				{
					return;
				}
			}
		}

		void checkin_all(CheckinHandler callable)
		{
			this->checkin(this->not_checked_in_n(), callable);
		}

		void checkin(size_t amount, CheckinHandler callable)
		{
			std::cout << "ChatLog : attempt to checkin " << amount << " logs..." << std::endl;
			if (amount > this->not_checked_in_n()) {
				throw CheckInTooMuchException();
			}
			this->curr_checkin_i.emplace(this->checkin_i);
			callable([amount, this]() mutable -> Message *
			{
				if ((*this->curr_checkin_i) < (this->checkin_i + amount))
				{
					return &(this->logs[(*this->curr_checkin_i)++]);
				}
				else {
					return {};
				}
			}, [amount, this]()
			{
				this->checkin_done(amount);
			});
		}

		void checkin_done(size_t amount)
		{
			std::cout << "ChatLog : report done checkin " << amount << " to db." << std::endl;
			if (amount > this->not_checked_in_n()) {
				throw CheckInTooMuchException();
			}
			this->checkin_i += amount;
			if(this->logs.size() >= this->size_limit)
			{
				this->release(this->checkin_i / 2);
			}
		}

		void auto_checkin(CheckinHandler handler)
		{
			this->checkin_handler.emplace(handler);
		}

		void add(const Message &message)
		{
			this->logs.push_back(message);
			if(this->checkin_handler.has_value())
			{
				if(this->not_checked_in_n() >= this->checkin_bundle_size)
				{
					this->checkin(this->checkin_bundle_size, *(this->checkin_handler));
				} else if(this->logs.size() >= this->size_limit)
				{
					this->release(this->checkin_i / 2);
				}
			}
		}

		void release(size_t amount)
		{
			std::cout << "ChatLog : released " << amount << std::endl;
			if(amount > this->checkin_i)
			{
				throw ReleaseTooMuchException();
			}
			this->logs.erase(this->logs.begin(), this->logs.begin() + amount);
			this->checkin_i -= amount;
		}

		size_t not_checked_in_n()
		{
			return (this->logs.size() - this->checkin_i);
		}

		NextMessageHandler log_revise()
		{
			if(this->logs.size() <= this->log_revise_size)
			{
				this->curr_revise_i.emplace(0);
			}
			else {
				this->curr_revise_i.emplace(logs.size() - log_revise_size);
			}
			return [this]() mutable->Message *
			{
				if ((*this->curr_revise_i) < this->logs.size())
				{
					return &(this->logs[(*this->curr_revise_i)++]);
				}
				else {
					return nullptr;
				}
			};
		}

		std::optional<size_t> curr_checkin_i;
		std::optional<size_t> curr_revise_i;
		size_t size_limit;
		size_t checkin_bundle_size;
		std::deque<Message> logs;
		size_t checkin_i;
		size_t log_revise_size;
		std::optional<CheckinHandler> checkin_handler;
	};

	template <typename SessionType> class DefaultSessionDeliverer
	{
	public:
		void operator() (SessionType &session, std::shared_ptr<chatroom::Message> message) {
			session.deliver(message);
		}
	};

	template <typename SessionType> class DefaultSessionCloser
	{
	public:
		void operator() (SessionType &session) {
			session.close();
		}
	};

	template <
		typename SessionType, 
		typename SessionDeliverer = DefaultSessionDeliverer<SessionType>,
		typename SessionCloser = DefaultSessionCloser<SessionType>
	> class Person
	{
	public:
		Person(const std::string &name)
			:name(name), login_info({})
		{
		}

		bool deliver_message(std::shared_ptr<chatroom::Message> message)
		{
			if (!this->login_info.has_value())
			{
				return false;
			}
			SessionDeliverer()(this->login_info->session, message);
			return true;
		}

		void logout()
		{
			this->login_info.reset();
		}

		void ban()
		{
			if (!this->login_info.has_value())
			{
				return;
			}
			SessionCloser()(this->login_info->session);
			logout();
		}

		class LoginInfo
		{
		public:
			LoginInfo(const std::string &name, SessionType& session, uint64_t unix_time=0)
			:login_unix_time(unix_time ? unix_time : std::time(NULL)),
			token_uuid(boost::uuids::random_generator()()),
			token(),
			session(session),
			name(name)
			{
				std::cout << "LoginInfo : " << name << " log in." << std::endl;
				this->token = boost::uuids::to_string(this->token_uuid);
			}

			~LoginInfo()
			{
				std::cout << "LoginInfo : " << name << " log out." << std::endl;
			}

			std::string name;
			uint64_t login_unix_time;
			std::string token;
			boost::uuids::uuid token_uuid;
			SessionType &session;
			
			bool verify(const std::string &token, const SessionType &session) const
			{
				return (this->token == token) &&
					(std::addressof(this->session) == std::addressof(session));
			}
		};
		std::string name;
		std::optional<LoginInfo> login_info;
	};

	CHAT_EXCEPTION_CLASS(UserNotFoundException, "user not found");
	CHAT_EXCEPTION_CLASS(AuthenticateFailedException, "authenticate failed");
	CHAT_EXCEPTION_CLASS(DuplicateLoginException, "duplicate login");
	CHAT_EXCEPTION_CLASS(UserBannedException, "user got banned");

	bool DefaultAuthenticateFunc(const std::string &name, const std::string &auth)
	{
		return false;
	}

	bool DefaultIsBannedFunc(const std::string &name)
	{
		return false;
	}

	template <typename SessionType>
	class Room
	{
	public:

		typedef typename std::function<bool(const std::string &, const std::string &)> AuthenticateFunc;
		typedef typename std::function<bool(const std::string &)> IsBannedFunc;

		Room(const std::string &room_id)
			: room_id(room_id), 
			authenticate_func(DefaultAuthenticateFunc), 
			is_banned_func(DefaultIsBannedFunc)
		{}

		void set_authenticate_func(AuthenticateFunc func)
		{
			this->authenticate_func = func;
		}

		bool authenticate(const std::string &name, const std::string &auth) const
		{
			return this->authenticate_func(name, auth);
		}

		void set_is_banned_func(IsBannedFunc func)
		{
			this->is_banned_func = func;
		}

		bool is_banned(const std::string &name) const
		{
			return this->is_banned_func(name);
		}

		void add_member(const std::string &name)
		{
			this->members.insert(std::make_pair(name, Person<SessionType>(name)));
		}
				
		std::string login(const std::string &name, const std::string &auth, SessionType &session)
		{
			if(members.count(name) == 0)
			{
				throw UserNotFoundException();
			}
			
			if (is_banned(name))
			{
				throw UserBannedException();
			}
			
			if (! authenticate(name, auth))
			{
				throw AuthenticateFailedException();
			}
			
			if (members.at(name).login_info.has_value())
			{
				throw DuplicateLoginException();
			}
			members.at(name).login_info.emplace(name, session);
			return members.at(name).login_info->token;
		}
		
		void logout(const std::string &name)
		{
			if (members.count(name))
			{
				members.at(name).logout();
			}
		}

		void ban(const std::string &name)
		{
			if (members.count(name))
			{
				members.at(name).ban();
			}
		}

		void unban(const std::string &name)
		{
			// Currently nothing to do
		}
		
		bool verify(const std::string &name, const std::string &token, SessionType &session)
		{
			if (members.count(name) == 0)
			{
				return false;
			}
			return members.at(name).login_info.has_value() &&
				(members.at(name).login_info->verify(token, session));
		}
		
		void send_message(const Message &message)
		{
			auto shared_message = std::make_shared<Message>(Message(message));
			std::for_each(
				members.begin(), members.end(),
				[this, shared_message, &message](auto &it)
				{
					try
					{
						it.second.deliver_message(shared_message);
					} catch (const std::exception &e)
					{
						std::cerr << "Room : error occurred while delivering message "
						          << message.id
						          << " to "
						          << it.first
						          << " : "
						          << std::endl;
						std::cerr << e.what() << std::endl;
					}
				});
		}
		
		template <typename HookedChatLog>
		void send_and_log_message(const Message &message, HookedChatLog &chat_log)
		{
			this->send_message(message);
			chat_log.add(message);
		}

		std::map<std::string, Person<SessionType> > members;
		std::string room_id;

		AuthenticateFunc authenticate_func;
		IsBannedFunc is_banned_func;
	};
}


