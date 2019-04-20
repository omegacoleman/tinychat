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
		Message(const std::string &sender, const std::string &text, uint64_t unix_time = 0)
			: sender(sender), text(text),
			unix_time(unix_time ? unix_time : std::time(NULL)),
			id_uuid(boost::uuids::random_generator()()),
		id()
		{
			this->id = boost::uuids::to_string(this->id_uuid);
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

		class LogIterator : public std::iterator<std::bidirectional_iterator_tag, const Message>
		{
		public:
			explicit LogIterator(ChatLog &chat_log, size_t index) : chat_log(chat_log), index(index) {}
			LogIterator& operator++() { index++; return *this; }
			LogIterator operator++(int) { LogIterator retval(chat_log, index); ++index; return retval; }
			bool operator==(LogIterator other) const { return index == other.index; }
			bool operator!=(LogIterator other) const { return index != other.index; }
			reference operator*() const { return chat_log.logs[index]; }
			Message *operator->() const { return &(chat_log.logs[index]); }

			size_t index;
			ChatLog &chat_log;
		};
		using checkin_range_callable = std::function<void(LogIterator, LogIterator, std::function<void()>)>;

		template <typename message_iterator>
		void load_from_db(message_iterator it, message_iterator end)
		{
			const size_t to_load_max = this->size_limit / 2;
			for(; it != end; ++it)
			{
				this->logs.push_front(*it);
				this->checkin_i++;
				if(this->logs.size() >= to_load_max)
				{
					return;
				}
			}
		}

		void checkin_all(checkin_range_callable callable)
		{
			this->checkin(this->not_checked_in_n(), callable);
		}

		void checkin(size_t amount, checkin_range_callable callable)
		{
			std::cout << "ChatLog : attempt to checkin " << amount << " logs..." << std::endl;
			if (amount > this->not_checked_in_n()) {
				throw CheckInTooMuchException();
			}
			callable(LogIterator(*this, this->checkin_i), LogIterator(*this, this->checkin_i + amount), [=]()
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

		void auto_checkin(checkin_range_callable handler)
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

		std::pair<LogIterator, LogIterator> log_revise()
		{
			if(this->logs.size() <= this->log_revise_size)
			{
				return std::make_pair(LogIterator(*this, 0), LogIterator(*this, logs.size()));
			}
			return std::make_pair(
				LogIterator(*this, logs.size() - this->log_revise_size),
				LogIterator(*this, logs.size()));
		}

		std::optional<checkin_range_callable> checkin_handler;
		size_t size_limit;
		size_t checkin_bundle_size;
		std::deque<Message> logs;
		size_t checkin_i;
		size_t log_revise_size;
	};

	template <typename session_type> class SessionDeliverer
	{
		public:
		void operator() (session_type &session, std::shared_ptr<chatroom::Message> message) {
			session.deliver(message);
		}
	};

	template<typename session_type, typename session_deliverer = SessionDeliverer<session_type> > class Person
	{
	public:
		Person(const std::string &name, const std::string &auth)
			:name(name), auth(auth), login_info({})
		{
		}

		bool authenticate(const std::string &name, const std::string &auth) const
		{
			return (this->name == name) && (this->auth == auth);
		}

		bool deliver_message(std::shared_ptr<chatroom::Message> message)
		{
			if(! this->login_info.has_value())
			{
				return false;
			}
			session_deliverer()(this->login_info->session, message);
			return true;
		}

		class LoginInfo
		{
		public:
			LoginInfo(const std::string &name, session_type& session, uint64_t unix_time=0)
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
			session_type& session;

			bool verify(const std::string &token, const session_type& session) const
			{
				return (this->token == token) &&
				(std::addressof(this->session) == std::addressof(session));
			}
		};
		std::string name;
		std::string auth;
		std::optional<LoginInfo> login_info;
	};

	CHAT_EXCEPTION_CLASS(UserNotFoundException, "user not found");
	CHAT_EXCEPTION_CLASS(AuthenticateFailedException, "authenticate failed");
	CHAT_EXCEPTION_CLASS(DuplicateLoginException, "duplicate login");

	template<typename session_type> class Room
	{
	public:
		template <typename UserAuthPairIterator>
		void load(UserAuthPairIterator it, UserAuthPairIterator end)
		{
			for (; it != end; ++it)
			{
				this->members.insert(std::make_pair(it->first, Person<session_type>(it->first, it->second)));
			}
		}

		std::string login(const std::string &name, const std::string &auth, session_type &session)
		{
			if(members.count(name) == 0)
			{
				throw UserNotFoundException();
			}
			if(! members.at(name).authenticate(name, auth))
			{
				throw AuthenticateFailedException();
			}
			if(members.at(name).login_info.has_value())
			{
				throw DuplicateLoginException();
			}
			members.at(name).login_info.emplace(name, session);
			return members.at(name).login_info->token;
		}

		void logout(const std::string &name)
		{
			members.at(name).login_info.reset();
		}

		void update_user(const std::string &name, const std::string &auth)
		{
			if (this->members.count(name))
			{
				this->members.at(name).auth = auth;
			}
			else {
				this->members.insert(std::make_pair(name, Person<session_type>(name, auth)));
			}
		}

		bool verify(const std::string &name, const std::string &token, session_type &session)
		{
			if(members.count(name) == 0)
			{
				return false;
			}
			return members.at(name).login_info.has_value() &&
			(members.at(name).login_info->verify(token, session));
		}

		void send_message(const Message &message)
		{
			auto shared_message = std::make_shared<Message>(Message(message));
			for(auto it = members.begin(); it != members.end(); ++it)
			{
				try
				{
					it->second.deliver_message(shared_message);
				} catch (const std::exception &e)
				{
					std::cerr << "Room : error occurred while delivering message " << message.id << " to " << it->first << " : " << std::endl;
					std::cerr << e.what() << std::endl;
				}
			}
		}

		template <typename HookedChatLog>
		void send_and_log_message(const Message &message, HookedChatLog &chat_log)
		{
			this->send_message(message);
			chat_log.add(message);
		}

		std::map<std::string, Person<session_type> > members;
	};
}


