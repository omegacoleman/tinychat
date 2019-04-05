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
	template<const char* info> class ChatException : public std::exception
	{
	public:
		virtual const char* what() const noexcept
		{
			return (std::string("error in chatroom.hpp : ") + info).c_str();
		}
	};

	template<typename session_type> class Person
	{
	public:
		Person(const std::string name, const std::string auth)
		:name(name), auth(auth), login_info({})
		{
		}
		
		bool authenticate(const std::string name, const std::string auth) const
		{
			return (this->name == name) && (this->auth == auth);
		}

		class LoginInfo
		{
		public:
			LoginInfo(std::string name, session_type& session, uint64_t unix_time=0)
			:login_unix_time(unix_time || std::time(NULL)), 
			token_uuid(boost::uuids::random_generator()()), 
			token(), 
			session(session)
			{
				this->token = boost::uuids::to_string(this->token_uuid);
			}
			uint64_t login_unix_time;
			std::string token;
			boost::uuids::uuid token_uuid;
			session_type& session;

			bool verify(const std::string token, const session_type& session) const
			{
				return (this->token == token) && 
				(std::addressof(this->session) == std::addressof(session));
			}
		};
		std::string name;
		std::string auth;
		std::optional<LoginInfo> login_info;
	};


	const char duplicate_login_err_msg[] = "duplicate login";
	const char authentication_failed_err_msg[] = "authenticate failed";
	const char user_not_found_err_msg[] = "user not found";

	using UserNotFoundException = ChatException<user_not_found_err_msg>;
	using AuthenticateFailedException = ChatException<authentication_failed_err_msg>;
	using DuplicateLoginException = ChatException<duplicate_login_err_msg>;

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

		std::string login(const std::string name, const std::string auth, session_type &session)
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

		bool verify(const std::string name, const std::string token, session_type &session)
		{
			if(members.count(name) == 0)
			{
				return false;
			}
			return members.at(name).login_info.has_value() &&
			(members.at(name).login_info->verify(token, session));
		}

		std::map<std::string, Person<session_type> > members;
	};
};


