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
#include "logging.hpp"

#include "db/via_bredis.hpp"
#include "db/dummy_client.hpp"

#include "tinyrpc/rpc_websocket_service.hpp"

#include "boost/program_options.hpp"

#include "optional_ssl_stream.hpp"
#include "lazy_timeout.hpp"
#include "boost_system_exception.hpp"

using namespace tinyrpc;

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;
using ws = websocket::stream<tinychat::utility::optional_ssl_stream<tcp::socket> >;

boost::asio::io_context ioc;

uint64_t sess_id = 0;

class rpc_session;

static std::unique_ptr<chatroom::Room<rpc_session> > room;
static std::unique_ptr<chatroom::ChatLog> chatlog;

static std::unique_ptr<tinychat::utility::lazy_timeout> timeout_scanner;

void serialize_message(chat::ChatMessage &to, const chatroom::Message &from)
{
	to.set_mtype(chat::message_simple);
	auto header = new chat::ChatMessageHeader();
	header->set_id(from.id);
	header->set_sender(from.sender);
	header->set_unix_time(from.unix_time);
	auto simple_message = new chat::ChatSimpleMessage();
	simple_message->set_allocated_header(header);
	simple_message->set_text(from.text);
	to.set_allocated_simple_message(simple_message);
}

class rpc_session : public std::enable_shared_from_this<rpc_session>
{
public:

	explicit rpc_session(ws&& s)
	: ws_(std::move(s)), 
		rpc_stub_(ws_), 
		id(sess_id++), 
		avail_flag(true)
	{
		auto &logger = tinychat::logging::logger::instance();
		this->uninst_lazy = timeout_scanner->bind([this]()
		{
			return this->flush_avail();
		}, [this]()
		{
			auto &logger = tinychat::logging::logger::instance();
			logger.info("session(lazy_timeout)") << "session " << identity() << " closed due to timeout.";
			return this->close();
		});
		logger.info("session") << "session #" << id << " opened.";
	}

	~rpc_session()
	{
		auto &logger = tinychat::logging::logger::instance();
		this->uninst_lazy();
		if(login_name)
			room->logout(login_name.value());
		logger.info("session") << "session " << identity() << " destructed.";
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
		REG_SVC(chat::ChatSimpleSendRequest, chat::ChatSimpleSendReply, chat_simple_send_service);
		REG_SVC(chat::GetLogRequest, chat::GetLogReply, get_log_service);

		boost::beast::multi_buffer buf;
		boost::system::error_code ec;

		while (ws_.is_open())
		{
			auto &logger = tinychat::logging::logger::instance();
			try
			{
				auto bytes = ws_.async_read(buf, yield[ec]); _RT_EC("read", ec);
				this->avail_flag = true;
				rpc_stub_.dispatch(buf, ec); _RT_EC("rpc_dispatch", ec);
				buf.consume(bytes);
			} catch (const std::exception &e)
			{
				logger.warning("session") << "session #" << id << " : " << e.what();
				return;
			}
		}
#undef REG_SVC
	}

	void login_service(const chat::LoginRequest& req, chat::LoginReply& reply)
	{
		auto &logger = tinychat::logging::logger::instance();
		try
		{
			logger.info("session.login_service") << req.name() << " wants to login. ";
			std::string token = room->login(req.name(), req.auth(), *this);
			reply.set_result(chat::login_result_ok);
			reply.set_token(token);
			login_name.emplace(req.name());
		}
		catch(const chatroom::UserNotFoundException &e) {
			reply.set_result(chat::login_result_not_registered);
			logger.info("session") << "login_service : " << e.what();
		}
		catch(const chatroom::AuthenticateFailedException &e) {
			reply.set_result(chat::login_result_auth_failed);
			logger.info("session") << "login_service : " << e.what();
		}
		catch(const chatroom::DuplicateLoginException &e) {
			reply.set_result(chat::login_result_duplicate_login);
			logger.info("session") << "login_service : " << e.what();
		}
		catch (const chatroom::UserBannedException &e) {
			reply.set_result(chat::login_result_banned);
			logger.info("session") << "login_service : " << e.what();
		}
		catch(const std::exception &e) {
			reply.set_result(chat::login_result_error);
			logger.warning("session") << "login_service : " << e.what();
		}
	}

	void verify_service(const chat::VerifyRequest& req, chat::VerifyReply& reply)
	{
		auto &logger = tinychat::logging::logger::instance();
		try
		{
			logger.info("session.verify_service") << identity() << " checked his status. ";
			if (this->login_name.has_value())
			{
				if (room->verify(*(this->login_name), req.token(), *this))
				{
					return;
				}
			}
		} catch(std::exception e) {
			logger.info("session") << "verify_service : " << e.what();
			throw e;
		}
	}

	void chat_simple_send_service(const chat::ChatSimpleSendRequest &req, chat::ChatSimpleSendReply &reply)
	{
		auto &logger = tinychat::logging::logger::instance();
		try
		{
			if (!this->login_name.has_value())
			{
				logger.info("session.chat_send_service") << identity() << " tried to send without login";
				reply.set_result(chat::send_result_error);
				return;
			}
			if (!room->verify(*(this->login_name), req.token(), *this))
			{
				logger.info("session.chat_send_service") << identity() << " tried to send something but fails the token valification";
				reply.set_result(chat::send_result_error);
				return;
			}

			chatroom::Message m(room->room_id, *(this->login_name), req.text());
			room->send_and_log_message(m, *chatlog);
			std::cout << "<" << *(this->login_name) << ">:" << req.text() << std::endl;
		} catch(const std::exception &e) {
			logger.warning("session") << "chat_send_service " << this->identity() << " : " << e.what();
			reply.set_result(chat::send_result_error);
			return;
		}
	}

	void get_log_service(const chat::GetLogRequest &req, chat::GetLogReply &reply)
	{
		auto &logger = tinychat::logging::logger::instance();
		try
		{
			if (!this->login_name.has_value())
			{
				logger.info("session.get_log_service") << identity() << " tried to get log without login";
				return;
			}
			if (!room->verify(*(this->login_name), req.token(), *this))
			{
				logger.info("session.get_log_service") << identity() << " tried to get log but fails the token valification";
				return;
			}
			auto iter_gen = chatlog->log_revise();
			for (auto it = iter_gen(); it != nullptr; it = iter_gen())
			{
				chat::ChatMessage *m = reply.add_chat_messages();
				serialize_message(*m, *it);
			}
		}
		catch (const std::exception &e) {
			logger.warning("session") << "get_log_service " << this->identity() << " : " << e.what();
			return;
		}
	}

	void deliver_proc(std::shared_ptr<chatroom::Message> message, boost::asio::yield_context yield)
	{
		boost::system::error_code ec;
		chat::NotifyChatMessageRequest req;
		chat::ChatMessage *chat_message = new chat::ChatMessage();
		serialize_message(*chat_message, *message);
		req.set_allocated_chat_message(chat_message);
		chat::NotifyChatMessageReply reply;
		this->rpc_stub_.async_call(req, reply, yield[ec]); _RT_EC("deliver_proc(" + this->identity() + ")", ec);
	}

	void deliver(std::shared_ptr<chatroom::Message> message)
	{
		boost::asio::spawn(ioc, [message, this](boost::asio::yield_context yield)
		{
			auto &logger = tinychat::logging::logger::instance();
			try
			{
				this->deliver_proc(message, yield);
			}
			catch (const std::exception &e)
			{
				logger.warning("session") << "deliver(" << message->id << ")->" << this->identity() << " : " << e.what();
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

	bool flush_avail()
	{
		bool ret = this->avail_flag;
		this->avail_flag = false;
		return ret;
	}

private:
	uint64_t id;
	ws ws_;
	rpc_websocket_service<ws> rpc_stub_;
	std::optional<std::string> login_name;

	bool avail_flag; // did the session read any data, until last flush_avail() call
	tinychat::utility::lazy_timeout::UninstallHandle uninst_lazy;
};

void do_session(tcp::socket &socket, 
	std::optional<boost::asio::ssl::context *> ssl_context, 
	boost::asio::yield_context yield)
{
	auto &logger = tinychat::logging::logger::instance();
	try
	{
		boost::system::error_code ec;

		std::optional<ws> s;
		
		if (ssl_context)
		{
			s.emplace(tinychat::utility::tag_ssl, std::move(socket), *(ssl_context.value()));
			s->next_layer().async_handshake(boost::asio::ssl::stream_base::server, yield[ec]); _RT_EC("ssl_handshake", ec);
		}
		else {
			s.emplace(tinychat::utility::tag_non_ssl, std::move(socket));
		}

		s->async_accept(yield[ec]); _RT_EC("ws_accept", ec);

		s->binary(true);

		auto ses = std::make_shared<rpc_session>(std::move(s.value()));
		ses->run(yield);
	}
	catch (const std::exception &e) {
		logger.error("do_session") << e.what();
	}
}

void do_listen(
	tcp::endpoint endpoint,
	std::optional<boost::asio::ssl::context *> ssl_context,
	boost::asio::yield_context yield)
{
	boost::system::error_code ec;
	auto &logger = tinychat::logging::logger::instance();

	tcp::acceptor acceptor(ioc);
	acceptor.open(endpoint.protocol(), ec); _RT_EC("acceptor.open", ec);
	acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec); _RT_EC("acceptor.set_option", ec);
	acceptor.bind(endpoint, ec); _RT_EC("acceptor.bind", ec);
	acceptor.listen(boost::asio::socket_base::max_listen_connections, ec); _RT_EC("acceptor.listen", ec);

	logger.info("do_listen") << "listening on ws://" << endpoint << "/";
	for(;;)
	{
		tcp::socket socket(ioc);
		acceptor.async_accept(socket, yield[ec]); _RT_EC("acceptor.async_accept", ec);
		boost::asio::spawn(ioc, [socket{ std::move(socket) }, ssl_context](
			boost::asio::yield_context yield) mutable
		{
			do_session(socket, ssl_context, yield);
		});
	}
}

int main(int argc, char* argv[])
{
	auto &logger = tinychat::logging::logger::instance();

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
			("heartbeat-kick", boost::program_options::value<unsigned int>()->default_value(40), "kick-offline connections idle(without heartbeat) for given seconds")
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

		std::unique_ptr<chatroom::db::via_bredis::bredis_client> db_client;
		if (vm.count("db-redis"))
		{
			
			boost::asio::ip::tcp::endpoint ep{
				boost::asio::ip::address::from_string(vm["db-redis"].as<std::string>()), 
				vm["redis-port"].as<unsigned short>()
			};
			db_client = std::make_unique<chatroom::db::via_bredis::bredis_client>(ioc, ep);
			
			chatlog->auto_checkin(
				[&db_client](chatroom::ChatLog::NextMessageHandler next, chatroom::ChatLog::DoneHander done_callback)
				{
					db_client->log_storage_checkin(next, done_callback);
				});
			
			db_client->user_name_auth().for_each_name([](const std::string &name)
			{
				room->add_member(name);
			});
			room->set_authenticate_func([&db_client](const std::string &name, const std::string &auth) -> bool
			{
				return db_client->authenticate(name, auth);
			});
			room->set_is_banned_func([&db_client](const std::string &name) -> bool
			{
				return db_client->is_banned(name);
			});
			db_client->user_name_auth().set_on_add_user([](const std::string & name)
			{
				room->add_member(name);
			});
			db_client->banlist().set_on_ban([](const std::string & name)
			{
				room->ban(name);
			});
			db_client->banlist().set_on_unban([](const std::string & name)
			{
				room->unban(name);
			});
		}
		else if (vm.count("db-dummy"))
		{
			std::for_each(chatroom::db::dummy::dummy_user_auth.begin(), chatroom::db::dummy::dummy_user_auth.end(), [](const auto & pair)
			{
				room->add_member(pair.first);
			});
			room->set_authenticate_func(chatroom::db::dummy::auth_func);
			room->set_is_banned_func(chatroom::db::dummy::is_banned);
			chatlog->auto_checkin(chatroom::db::dummy::dummy_db_checkin<chatroom::ChatLog::NextMessageHandler, chatroom::ChatLog::DoneHander>);
		}
		else
		{
			logger.error("main") << "must at least specify one db client, check --help for more";
			return EXIT_FAILURE;
		}

		timeout_scanner = std::make_unique<tinychat::utility::lazy_timeout>(std::chrono::seconds(vm["heartbeat-kick"].as<unsigned int>()));
		timeout_scanner->start_scan(ioc);

		boost::asio::spawn(ioc, [&address, &port, &ssl_context](boost::asio::yield_context yield)
		{
			auto &logger = tinychat::logging::logger::instance();
			try
			{
				do_listen(tcp::endpoint{ address, port }, 
					ssl_context ? &(*ssl_context) : 
					(std::optional<boost::asio::ssl::context *>{}), yield);
			}
			catch (const std::exception &e)
			{
				logger.error("do_listen") << e.what();
			}
		});

		ioc.run();

		return EXIT_SUCCESS;
	}
	catch (const std::exception &e)
	{
		logger.error("main") << e.what();
		return EXIT_FAILURE;
	}
}
