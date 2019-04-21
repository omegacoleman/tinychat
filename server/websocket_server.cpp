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
#include "boost/asio/ssl.hpp"

#include "boost/beast.hpp"

#include "chat.pb.h"

#include "chatroom.hpp"

#include "db/bredis_client.hpp"
#include "db/dummy_client.hpp"

#include "tinyrpc/rpc_websocket_service.hpp"
#include "boost_system_exception.hpp"

#include "boost/program_options.hpp"

#include "optional_ssl_stream.hpp"

using namespace tinyrpc;


using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;
using ws = websocket::stream<tinychat::utility::optional_ssl_stream<tcp::socket> >;

boost::asio::io_context ioc;

uint64_t sess_id = 0;

class rpc_session;

static std::unique_ptr<chatroom::Room<rpc_session> > room;
static std::unique_ptr<chatroom::ChatLog> chatlog;

class rpc_session : public std::enable_shared_from_this<rpc_session>
{
public:

	explicit rpc_session(ws&& s)
	: ws_(std::move(s)), 
		rpc_stub_(ws_), 
		id(sess_id++)
	{
		std::cout << "rpc_session opened, id #" << id << std::endl;
	}

	~rpc_session()
	{
		if(login_name)
			room->logout(login_name.value());
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
		REG_SVC(chat::GetLogRequest, chat::GetLogReply, get_log_service);

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
		} catch (const chatroom::UserBannedException &e) {
			reply.set_state(chat::LoginReply::banned);
			std::cerr << e.what() << std::endl;
		}
		catch(const std::exception &e) {
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
			chatroom::Message m(room->room_id, req.name(), req.text());
			room->send_and_log_message(m, *chatlog);
			std::cout << "|| " << req.name() << " : " << req.text() << std::endl;
		} catch(const std::exception &e) {
			std::cerr << e.what() << std::endl;
			reply.set_result(chat::ChatSendReply::error);
			return;
		}
	}

	void get_log_service(const chat::GetLogRequest &req, chat::GetLogReply &reply)
	{
		if (!room->verify(req.name(), req.token(), *this))
		{
			std::cerr << "name " << req.name() << " tried to get log but fails the token valification" << std::endl;
			return;
		}
		auto iter_pair = chatlog->log_revise();
		for (auto it = iter_pair.first; it != iter_pair.second; ++it)
		{
			chat::ChatMessage *m = reply.add_chat_messages();
			m->set_id(it->id);
			m->set_sender(it->sender);
			m->set_text(it->text);
			m->set_unix_time(it->unix_time);
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
		return login_name.value_or("#" + std::to_string(id));
	}

	void close()
	{
		ws_.lowest_layer().cancel();
	}

private:
	uint64_t id;
	ws ws_;
	rpc_websocket_service<ws> rpc_stub_;
	std::optional<std::string> login_name;
};

void do_session(tcp::socket &socket, 
	std::optional<boost::asio::ssl::context *> ssl_context, 
	boost::asio::yield_context yield)
{
	try
	{
		boost::system::error_code ec;

		std::optional<ws> s;
		
		if (ssl_context)
		{
			s.emplace(tinychat::utility::tag_ssl, std::move(socket), *(ssl_context.value()));
			s->next_layer().async_handshake(boost::asio::ssl::stream_base::server, yield[ec]);
			if (ec)
				throw tinychat::utility::boost_system_ec_exception(ec);
		}
		else {
			s.emplace(tinychat::utility::tag_non_ssl, std::move(socket));
		}

		s->async_accept(yield[ec]);
		if (ec)
			throw tinychat::utility::boost_system_ec_exception(ec);

		s->binary(true);

		auto ses = std::make_shared<rpc_session>(std::move(s.value()));
		ses->run(yield);
	}
	catch (const std::exception &e) {
		std::cerr << "do_session failed once, reason : " << e.what() << std::endl;
	}
}

void do_listen(
	tcp::endpoint endpoint,
	std::optional<boost::asio::ssl::context *> ssl_context,
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
			boost::asio::spawn(ioc, [socket{ std::move(socket) }, ssl_context](
				boost::asio::yield_context yield) mutable
			{
				do_session(socket, ssl_context, yield);
			});
		}
	}
}

int main(int argc, char* argv[])
{
	try
	{
		boost::program_options::options_description desc{ "Options" };
		desc.add_options()
			("help", "Print help message")
			("host", boost::program_options::value<std::string>()->default_value("0.0.0.0"), "The host to listen")
			("port", boost::program_options::value<unsigned short>()->default_value(8000), "The port to listen")
			("chatlog-limit", boost::program_options::value<size_t>()->default_value(400), "ChatLog's max entries limit")
			("chatlog-checkin", boost::program_options::value<size_t>()->default_value(10), "ChatLog's db checkin bundle size")
			("chatlog-revise", boost::program_options::value<size_t>()->default_value(50), "ChatLog's max entries sent to client upon log request")
			("db-redis", boost::program_options::value<std::string>(), "Connect to redis at specified host as database")
			("redis-port", boost::program_options::value<unsigned short>()->default_value(6379), "Redis database port, use with --db-redis")
			("db-dummy", "Connect to an internal fake database for testing")
			("use-ssl", "use SSL encryption, thus use websocket over https")
			("ssl-cert", boost::program_options::value<std::string>(), "cert file path, required with --use-ssl")
			("ssl-key", boost::program_options::value<std::string>(), "private key file path, required with --use-ssl")
			("room-id", boost::program_options::value<std::string>()->default_value("tc"), "chatroom unique id, change it to different"
				" only when you have parallel tinychat processes running on same db")
			;
		boost::program_options::variables_map vm;
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
		boost::program_options::notify(vm);
		if (vm.count("help"))
		{
			std::cerr << desc << std::endl;
			return EXIT_FAILURE;
		}

		room = std::make_unique<chatroom::Room<rpc_session> >(vm["room-id"].as<std::string>());
		
		chatlog = std::make_unique<chatroom::ChatLog>(
			vm["chatlog-limit"].as<size_t>(), 
			vm["chatlog-checkin"].as<size_t>(), 
			vm["chatlog-revise"].as<size_t>());

		auto const address = boost::asio::ip::address::from_string(vm["host"].as<std::string>() );
		auto const port = vm["port"].as<unsigned short>();

		std::unique_ptr<chatroom::db::via_bredis::connection> db_conn;

		std::unique_ptr<boost::asio::ssl::context> ssl_context;

		if (vm.count("use-ssl"))
		{
			namespace ssl = boost::asio::ssl;
			ssl_context = std::make_unique<ssl::context>(
				ssl::context::method::tlsv12_server);
			ssl_context->use_certificate_file(vm["ssl-cert"].as<std::string>(),
				ssl::context::file_format::pem);
			ssl_context->use_private_key_file(vm["ssl-key"].as<std::string>(),
				ssl::context::file_format::pem);
			ssl_context->set_verify_mode(ssl::context::verify_peer);
			ssl_context->set_options(ssl::context::default_workarounds);
			ssl_context->set_options(ssl::context::single_dh_use);
		}

		if (vm.count("db-redis"))
		{
			db_conn = std::make_unique<chatroom::db::via_bredis::connection>(
				ioc,
				vm["db-redis"].as<std::string>(),
				vm["redis-port"].as<unsigned short>());
			
			
			auto users = db_conn->fetch_users();
			room->load_user(users.begin(), users.end());
			auto banned_users = db_conn->fetch_banned_users();
			room->load_banned_user(banned_users.begin(),banned_users.end());
			
			db_conn->auto_refresh_users(
				[](const std::string &name, const std::string &auth)
				{
					room->update_user(name, auth);
				});
			
			db_conn->auto_refresh_banned_users(
				[](std::string user)
				{
					room->ban(user);
				});
			
			chatlog->auto_checkin(
				[&db_conn](chatroom::ChatLog::LogIterator it, chatroom::ChatLog::LogIterator end, std::function<void()> done_callback)
				{
					db_conn->checkin_log(it, end, done_callback);
				});
			
		}
		else if (vm.count("db-dummy"))
		{
			room->load_user(chatroom::db::dummy::dummy_user_auth.begin(), chatroom::db::dummy::dummy_user_auth.end());
			chatlog->auto_checkin(chatroom::db::dummy::dummy_db_checkin);
		}
		else
		{
			std::cerr << "error : must at least specify one db client, check --help for more" << std::endl;
			return EXIT_FAILURE;
		}

		boost::asio::spawn(ioc, [&address, &port, &ssl_context](boost::asio::yield_context yield)
		{
			try
			{
				do_listen(tcp::endpoint{ address, port }, 
					ssl_context ? &(*ssl_context) : 
					(std::optional<boost::asio::ssl::context *>{}), yield);
			}
			catch (const std::exception &e)
			{
				std::cerr << "do_listen failed : " << e.what() << std::endl;
			}
		});

		ioc.run();

		return EXIT_SUCCESS;
	}
	catch (const std::exception &e)
	{
		std::cerr << "error : " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
