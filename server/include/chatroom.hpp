#pragma once

#include <map>
#include <ctime>
#include <utility>
#include <memory>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace chatroom
{
	#define CHAT_EXCEPTION_STRING(info) (("error in chatroom.hpp : " info))
	class ChatException : public std::exception
	{
	public:
		virtual const char* what() const noexcept
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
			LoginInfo(std::string name, session_type& session, uint64_t unix_time=0)
			:login_unix_time(unix_time ? unix_time : std::time(NULL)),
			token_uuid(boost::uuids::random_generator()()),
			token(),
			session(session),
			error_clear(true)
			{
				this->token = boost::uuids::to_string(this->token_uuid);
			}
			uint64_t login_unix_time;
			std::string token;
			boost::uuids::uuid token_uuid;
			session_type& session;
			bool error_clear;

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
		Room()
		:members()
		{}

		static std::unique_ptr<Room> load_room() // simulate loading from database
		{
			std::unique_ptr<Room> room = std::make_unique<Room>();
			room->members.insert(std::make_pair("youcai", Person<session_type>("youcai", "66666")));
			room->members.insert(std::make_pair("orange", Person<session_type>("orange", "77777")));
			room->members.insert(std::make_pair("kim", Person<session_type>("kim", "88888")));
			return room;
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
					std::cerr << "error occured while delivering message " << message.id << " to " << it->first << " : " << std::endl;
					std::cerr << e.what() << std::endl;
					it->second.login_info->error_clear = false;
				}
			}
		}

		std::map<std::string, Person<session_type> > members;
	};
};


