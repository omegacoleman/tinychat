#include "boost/beast.hpp"
#include "boost/beast/websocket.hpp"
#include "boost/asio.hpp"
#include "boost/asio/spawn.hpp"
#include "boost/thread.hpp"
#include "boost/program_options.hpp"
#include "boost/asio/ssl.hpp"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>

#include "chat.pb.h"

#include "tinyrpc/rpc_websocket_service.hpp"

#include "utility/async_stdio.hpp"
#include "utility/boost_system_exception.hpp"
#include "utility/worded_exception.hpp"

#include "utility/optional_ssl_stream.hpp"

#include "utility/logging.hpp"

boost::program_options::variables_map arg_vm;

using namespace tinyrpc;

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

using ws = websocket::stream<
	tinychat::utility::optional_ssl_stream<tcp::socket> >;

void show_chat_message(const chat::ChatMessage& chat_message)
{
	auto &logger = tinychat::logging::logger::instance();
	if (chat_message.mtype() == chat::message_simple)
	{
		auto simple_message = chat_message.simple_message();
		std::cout << "<" << simple_message.header().sender() << ">:"
			<< simple_message.text() << std::endl;
	}
	else {
		logger.warning("show_chat_message") << "(Got a message but the format is currently unsupported)";
	}
}

class rpc_session : public std::enable_shared_from_this<rpc_session>
{
	public:
		rpc_session(ws &&s)
			: ws_(std::move(s)),
				rpc_stub_(ws_)
		{}

		~rpc_session() = default;
		
		void run(boost::asio::yield_context yield)
		{
			boost::beast::multi_buffer buf;
			boost::system::error_code ec;

			while ( true )
			{
				auto bytes = ws_.async_read(buf, yield[ec]); _RT_EC("read", ec);
				rpc_stub_.dispatch(buf, ec); _RT_EC("rpc_dispatch", ec);
				buf.consume(bytes);
			}
		}

		void notify_chat_message_service(const chat::NotifyChatMessageRequest &req,
				chat::NotifyChatMessageReply &reply)
		{
			show_chat_message(req.chat_message());
		}
		
		void start_timed_heartbeat(
			boost::asio::io_context &ioc,
			const std::string &name,
			const std::string &token,
			std::chrono::duration<std::size_t> duration
			)
		{
			spawn(ioc, [duration, name, token, &ioc, this](boost::asio::yield_context yield)
			{
				auto& logger = tinychat::logging::logger::instance();
				try
				{
					boost::asio::steady_timer timer(ioc);
					chat::VerifyRequest v_req;
					v_req.set_token(token);
					boost::system::error_code ec;
					while (true)
					{
						timer.expires_after(duration);
						timer.async_wait(yield[ec]); _RT_EC("wait(hb)", ec);
						chat::VerifyReply v_reply;
						rpc_stub_.async_call(v_req, v_reply, yield[ec]); _RT_EC("rpc_call(hb)", ec);
					}
				}
				catch (const std::exception &e) {
					logger.error("heartbeat") << "error : " << e.what();
					_exit(EXIT_FAILURE);
				}
			});
		}

		void chat_proc(boost::asio::yield_context yield, boost::asio::io_context &ioc)
		{
			auto &logger = tinychat::logging::logger::instance();
			rpc_stub_.rpc_bind<chat::NotifyChatMessageRequest, chat::NotifyChatMessageReply>(
				[this] (const chat::NotifyChatMessageRequest &req, chat::NotifyChatMessageReply &reply)
			{
				this->notify_chat_message_service(req, reply);
			});


			boost::system::error_code ec;

			chat::LoginRequest msg;

			msg.set_name(arg_vm["user"].as<std::string>());
			msg.set_auth(arg_vm["auth"].as<std::string>());

			chat::LoginReply reply;

			rpc_stub_.async_call(msg, reply, yield[ec]); _RT_EC("rpc_call(login)", ec);

			if ( reply.result() != chat::login_result_ok )
			{
				switch ( reply.result())
				{
					case (chat::login_result_not_registered):
						logger.error("login") << "name not registered.";
						break;
					case (chat::login_result_auth_failed):
						logger.error("login") << "authentication failed.";
						break;
					case (chat::login_result_duplicate_login):
						logger.error("login") << "this account is already online. contact the admin.";
						break;
					case (chat::login_result_banned):
						logger.error("login") << "you got banned.";
					default:
						logger.error("login") << "error occured during login. quit.";
						break;
				}
				ws_.close({}, ec);
				return;
			}

			logger.info("login") << "login succeed.";
			std::string token = reply.token();
			long long hb_interval = arg_vm["heartbeat"].as<long long>();
			if (hb_interval > 0)
				start_timed_heartbeat(ioc, arg_vm["user"].as<std::string>(), token, std::chrono::seconds(hb_interval));
			logger.info("login") << "your token : " << token;


			chat::GetLogRequest gl_req;
			gl_req.set_token(token);
			chat::GetLogReply gl_reply;
			rpc_stub_.async_call(gl_req, gl_reply, yield[ec]); _RT_EC("rpc_call(getlog)", ec);
			if (gl_reply.chat_messages_size())
			{
				// std::cout << "they said these before you join : " << std::endl;
				std::cout << "LOGREVISE------------------" << std::endl;
				for (int i = 0; i < gl_reply.chat_messages_size(); i++)
				{
					auto it = gl_reply.chat_messages(i);
					show_chat_message(it);
				}
				std::cout << "LOGREVISE------------------" << std::endl;
			}
			logger.info("session") << "prompting started.";

			while ( true )
			{
				auto context = tinychat::utility::async_stdin_getline(ioc, yield[ec]); _RT_EC("getline", ec);
				chat::ChatSimpleSendRequest v_req;
				chat::ChatSimpleSendReply v_reply;
				v_req.set_token(token);
				v_req.set_text(context);
				rpc_stub_.async_call(v_req, v_reply, yield[ec]); _RT_EC("rpc_call(chatsend)", ec);
				if ( v_reply.result() == chat::send_result_ok )
				{
					logger.info("session") << "message sent";
				}
				else
				{
					logger.info("warning") << "message failed to send";
				}
			}
		}

	private:
		ws ws_;
		rpc_websocket_service<ws> rpc_stub_;
};

void do_session(
		std::string const &host,
		std::string const &port,
		boost::asio::io_context &ioc,
		std::optional<boost::asio::ssl::context *> ssl_context,
		boost::asio::yield_context yield)
{
	boost::system::error_code ec;

	tcp::resolver resolver{ioc};
	std::optional<ws> s;
	if (ssl_context)
	{
		s.emplace(tinychat::utility::tag_ssl, ioc, *(ssl_context.value()));
	} else
	{
		s.emplace(tinychat::utility::tag_non_ssl, ioc);
	}

	auto const results = resolver.async_resolve(host, port, yield[ec]); _RT_EC("resolver.async_resolve", ec);

	boost::asio::async_connect(s->next_layer().next_layer(), results.begin(), results.end(),
			yield[ec]); _RT_EC("async_connect", ec);

	if (ssl_context)
	{
		s->next_layer().async_handshake(boost::asio::ssl::stream_base::client, yield[ec]); _RT_EC("ssl_handshake", ec);
	}

	s->async_handshake(host, "/tinychat", yield[ec]); _RT_EC("ws_handshake", ec);
	s->binary(true);

	auto ses = std::make_shared<rpc_session>(std::move(s.value()));
	boost::asio::spawn(ioc, [&ses] (boost::asio::yield_context yield)
	{
		auto &logger = tinychat::logging::logger::instance();
		try
		{
			ses->run(yield);
		}
		catch (const std::exception &e) {
			logger.error("do_session") << "error in session : " << e.what();
			_exit(EXIT_FAILURE);
		}
	});
	ses->chat_proc(yield, ioc);
}

int main(int argc, char **argv)
{
	auto &logger = tinychat::logging::logger::instance();
	try
	{
		boost::program_options::options_description desc("Options");
		desc.add_options()
			("help", "Print help message")
			("server", boost::program_options::value<std::string>(), "Server address")
			("port", boost::program_options::value<std::string>()->default_value("8000"), "Server port")
			("user", boost::program_options::value<std::string>(), "user field for login")
			("auth", boost::program_options::value<std::string>(), "auth field for login, may require hashing")
			("heartbeat", boost::program_options::value<long long>()->default_value(30),
				"interval to send heartbeat packet, 0 means no heartbeat")
			("use-ssl", "use SSL encryption, thus use websocket over https")
			("root-cert", boost::program_options::value<std::string>(), "root verify cert file path, required with --use-ssl")
			;

		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), arg_vm);
		boost::program_options::notify(arg_vm);

		if (arg_vm.count("help"))
		{
			std::cerr << desc << std::endl;
			return EXIT_FAILURE;
		}

		if (!arg_vm.count("server"))
		{
			logger.error("main") << "error : --server not specified, use --help for usage";
			return EXIT_FAILURE;
		}

		if (!arg_vm.count("user"))
		{
			logger.error("main") << "error : --user not specified, use --help for usage";
			return EXIT_FAILURE;
		}

		if (!arg_vm.count("auth"))
		{
			logger.error("main") << "error : --auth not specified, use --help for usage";
			return EXIT_FAILURE;
		}

		std::unique_ptr<boost::asio::ssl::context> ssl_context;

		if (arg_vm.count("use-ssl"))
		{
			namespace ssl = boost::asio::ssl;
			ssl_context = std::make_unique<ssl::context>(
				ssl::context::method::tlsv12_client);
			ssl_context->load_verify_file(arg_vm["root-cert"].as<std::string>());
			ssl_context->set_verify_mode(ssl::context::verify_peer);
			ssl_context->set_options(ssl::context::default_workarounds);
			ssl_context->set_options(ssl::context::single_dh_use);
		}

		auto const host = argv[1];
		auto const port = argv[2];

		boost::asio::io_context ioc;

		boost::asio::spawn(ioc, [&ioc, &ssl_context](boost::asio::yield_context yield)
		{
			try
			{
				do_session(
					arg_vm["server"].as<std::string>(),
					arg_vm["port"].as<std::string>(),
					ioc, ssl_context ? &(*ssl_context) :
					(std::optional<boost::asio::ssl::context *>{}), 
					yield
				);
			}
			catch (const std::exception &e) {
				auto &logger = tinychat::logging::logger::instance();
				logger.error("do_session") << "error creating session : " << e.what();
				_exit(EXIT_FAILURE);
			}
		});

		ioc.run();

		return EXIT_SUCCESS;
	}
	catch (const std::exception &e) {
		logger.error("do_error") << "error in main process : " << e.what();
		return EXIT_FAILURE;
	}
}

