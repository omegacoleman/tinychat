#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "boost/asio.hpp"
#include "boost/asio/spawn.hpp"

#include "chat.pb.h"

#include "chatroom.hpp"

#include "db/bredis_client.hpp"
#include "db/dummy_client.hpp"

#include "tinyrpc/rpc_websocket_service.hpp"
#include "boost_system_exception.hpp"

#include "boost/program_options.hpp"

using namespace tinyrpc;


using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

using ws = websocket::stream<tcp::socket>;

boost::asio::io_context ioc;

class rpc_session;

std::unique_ptr<chatroom::Room<rpc_session> > room;
std::unique_ptr<chatroom::ChatLog > chatlog;

uint64_t sess_id = 0;

class rpc_session : public std::enable_shared_from_this<rpc_session>
{
public:
	rpc_session(ws&& s)
	: ws_(std::move(s)),
		rpc_stub_(ws_), 
		id(sess_id++)
	{
		std::cout << "rpc_session opened, id #" << id << std::endl;
	}

	~rpc_session()
	{
		if(login_name) room->logout(login_name.value());
		std::cout << "rpc_session destructing #" << id << std::endl;
	}

	void run(boost::asio::yield_context yield)
	{
#define REG_SVC(type_req, type_res, func_name) rpc_stub_.rpc_bind<type_req, type_res>(\
			[this](const type_req &req, type_res &reply)\
		{\
			this->func_name(req, reply);\
		});
		REG_SVC(chat::LoginRequest, chat::LoginReply, login_service);
		REG_SVC(chat::VerifyRequest, chat::VerifyReply, verify_service);
		REG_SVC(chat::ChatSendRequest, chat::ChatSendReply, chat_send_service);

		boost::beast::multi_buffer buf;
		boost::system::error_code ec;

		while (ws_.is_open())
		{
			try
			{
				auto bytes = ws_.async_read(buf, yield[ec]);
				if (ec)
					throw tinychat::utility::boost_system_ec_exception(ec);
				rpc_stub_.dispatch(buf, ec);
				if (ec)
					throw tinychat::utility::boost_system_ec_exception(ec);
				buf.consume(bytes);
			} catch (const std::exception &e)
			{
				std::cerr << "session #" << id << " error : " << e.what() << std::endl;
				return;
			}
		}
#undef REG_SVC
	}

	void login_service(const chat::LoginRequest& req, chat::LoginReply& reply)
	{
		try
		{
			std::cout << req.name() << " wants to login. " << std::endl;
			std::string token = room->login(req.name(), req.auth(), *this);
			reply.set_state(chat::LoginReply::ok);
			reply.set_token(token);
			login_name.emplace(req.name());
		} catch(const chatroom::UserNotFoundException &e) {
			reply.set_state(chat::LoginReply::not_registered);
			std::cerr << e.what() << std::endl;
		} catch(const chatroom::AuthenticateFailedException &e) {
			reply.set_state(chat::LoginReply::auth_failed);
			std::cerr << e.what() << std::endl;
		} catch(const chatroom::DuplicateLoginException &e) {
			reply.set_state(chat::LoginReply::duplicate_login);
			std::cerr << e.what() << std::endl;
		} catch(const std::exception &e) {
			reply.set_state(chat::LoginReply::error);
			std::cerr << e.what() << std::endl;
		}
	}

	void verify_service(const chat::VerifyRequest& req, chat::VerifyReply& reply)
	{
		try
		{
			std::cout << req.name() << " checked his status. " << std::endl;
			reply.set_ok(room->verify(req.name(), req.token(), *this));
		} catch(std::exception e) {
			reply.set_ok(false);
			std::cerr << e.what() << std::endl;
		}
	}

	void chat_send_service(const chat::ChatSendRequest &req, chat::ChatSendReply &reply)
	{
		if(!room->verify(req.name(), req.token(), *this))
		{
			std::cerr << "name " << req.name() << " tried to send something but fails the token valification" << std::endl;
			reply.set_result(chat::ChatSendReply::error);
			return;
		}
		try
		{
			chatroom::Message m(req.name(), req.text());
			room->send_and_log_message(m, *chatlog);
			std::cout << "|| " << req.name() << " : " << req.text() << std::endl;
		} catch(const std::exception &e) {
			std::cerr << e.what() << std::endl;
			reply.set_result(chat::ChatSendReply::error);
			return;
		}
	}

	void deliver_proc(std::shared_ptr<chatroom::Message> message, boost::asio::yield_context yield)
	{
		boost::system::error_code ec;
		chat::NotifyChatMessageRequest req;
		chat::ChatMessage *chat_message = new chat::ChatMessage();
		chat_message->set_id(message->id);
		chat_message->set_sender(message->sender);
		chat_message->set_unix_time(message->unix_time);
		chat_message->set_text(message->text);
		req.set_allocated_chat_message(chat_message);
		chat::NotifyChatMessageReply reply;
		this->rpc_stub_.async_call(req, reply, yield[ec]);
		if(ec)
		{
			throw tinychat::utility::boost_system_ec_exception(ec);
		}
	}

	void deliver(std::shared_ptr<chatroom::Message> message)
	{
		boost::asio::spawn(ioc, [message, this](boost::asio::yield_context yield)
		{
			try
			{
				this->deliver_proc(message, yield);
			}
			catch (const std::exception &e) {
				std::cerr << "error occured delivering message " << message->id << " to " << this->identity() << " : " << e.what() << std::endl;
			}
		});
	}

	std::string identity()
	{
		return login_name.value_or("#" + id);
	}

private:
	uint64_t id;
	ws ws_;
	rpc_websocket_service<ws> rpc_stub_;
	std::optional<std::string> login_name;
};

void do_session(tcp::socket& socket)
{
	boost::asio::spawn(ioc, [&socket](boost::asio::yield_context yield)
	{
		try
		{
			boost::system::error_code ec;

			ws s{ std::move(socket) };

			s.async_accept(yield[ec]);
			if (ec)
				throw tinychat::utility::boost_system_ec_exception(ec);

			s.binary(true);

			auto ses = std::make_shared<rpc_session>(std::move(s));
			ses->run(yield);
		}
		catch (const std::exception &e) {
			std::cerr << "do_session failed once, reason : " << e.what() << std::endl;
		}
	});
}

void do_listen(
	tcp::endpoint endpoint,
	boost::asio::yield_context yield)
{
	boost::system::error_code ec;

	tcp::acceptor acceptor(ioc);
	acceptor.open(endpoint.protocol(), ec);
	if(ec)
		throw tinychat::utility::boost_system_ec_exception(ec);

	acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
	if(ec)
		throw tinychat::utility::boost_system_ec_exception(ec);

	acceptor.bind(endpoint, ec);
	if(ec)
		throw tinychat::utility::boost_system_ec_exception(ec);

	acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
	if(ec)
		throw tinychat::utility::boost_system_ec_exception(ec);

	std::cout << "listening on ws://" << endpoint << "/" << std::endl;
	for(;;)
	{
		tcp::socket socket(ioc);
		acceptor.async_accept(socket, yield[ec]);
		if (ec)
		{
			throw tinychat::utility::boost_system_ec_exception(ec);
		}
		else
		{
			do_session(std::move(socket));
		}
	}
}

int main(int argc, char* argv[])
{
	room = std::make_unique<chatroom::Room<rpc_session> >();
	chatlog = std::make_unique<chatroom::ChatLog>(30, 10, 14);

	boost::program_options::options_description desc{ "Options" };
	desc.add_options()
		("help", "Print help message")
		("host", boost::program_options::value<std::string>()->default_value("0.0.0.0"), "The host to listen")
		("port", boost::program_options::value<unsigned short>()->default_value(8000), "The port to listen")
		("db-redis", boost::program_options::value<std::string>(), "Connect to redis at specified host as database")
		("redis-port", boost::program_options::value<unsigned short>()->default_value(6379), "Redis database port, use with --db-redis")
		("db-dummy", "Connect to an internal fake database for testing")
		;
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);
	if (vm.count("help"))
	{
		std::cerr << desc << std::endl;
		return EXIT_FAILURE;
	}

	auto const address = boost::asio::ip::address::from_string(vm["host"].as<std::string>() );
	auto const port = vm["port"].as<unsigned short>();

	std::unique_ptr<chatroom::db::via_bredis::connection> db_conn;

	if (vm.count("db-redis"))
	{
		db_conn = std::make_unique<chatroom::db::via_bredis::connection>(
			ioc,
			vm["db-redis"].as<std::string>(),
			vm["redis-port"].as<unsigned short>());
		db_conn->reload_users_sync();
		auto &user_auth_it_pair = db_conn->users();
		room->load(user_auth_it_pair.first, user_auth_it_pair.second);
		db_conn->auto_refresh_users([](const std::string &name, const std::string &auth) {
			room->update_user(name, auth);
		});
		chatlog->auto_checkin([&db_conn](chatroom::LogIterator it, chatroom::LogIterator end, std::function<void()> done_callback) {
			db_conn->checkin_log(it, end, done_callback);
		});
	}
	else if (vm.count("db-dummy"))
	{
		room->load(chatroom::db::dummy::dummy_user_auth.begin(), chatroom::db::dummy::dummy_user_auth.end());
		chatlog->auto_checkin(chatroom::db::dummy::dummy_db_checkin);
	}
	else
	{
		std::cerr << "error : must at least specify one db client, check --help for more" << std::endl;
		return EXIT_FAILURE;
	}

	boost::asio::spawn(ioc, [&address, &port](boost::asio::yield_context yield)
	{
		try
		{
			do_listen(tcp::endpoint{ address, port }, yield);
		}
		catch (const std::exception &e)
		{
			std::cerr << "do_listen failed : " << e.what() << std::endl;
		}
	});

	ioc.run();

	return EXIT_SUCCESS;
}
