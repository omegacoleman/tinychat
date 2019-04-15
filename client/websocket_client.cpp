#include "boost/beast.hpp"
#include "boost/beast/websocket.hpp"
#include "boost/asio.hpp"
#include "boost/asio/spawn.hpp"
#include "boost/thread.hpp"
#include "boost/program_options.hpp"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>

#include "chat.pb.h"

#include "tinyrpc/rpc_websocket_service.hpp"

#include "async_stdio.hpp"
#include "boost_system_exception.hpp"
#include "worded_exception.hpp"

boost::program_options::variables_map arg_vm;

using namespace tinyrpc;

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

using ws = websocket::stream<tcp::socket>;

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
				auto bytes = ws_.async_read(buf, yield[ec]);
				if (ec)
				{
					throw tinychat::utility::boost_system_ec_exception(ec);
				}
				rpc_stub_.dispatch(buf, ec);
				if (ec)
				{
					throw tinychat::utility::boost_system_ec_exception(ec);
				}
				buf.consume(bytes);
			}
		}

		void notify_chat_message_service(const chat::NotifyChatMessageRequest &req,
				chat::NotifyChatMessageReply &reply)
		{
			std::cout << "|| " << req.chat_message().sender() << " : "
				<< req.chat_message().text() << std::endl;
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
				try
				{
					boost::asio::steady_timer timer(ioc);
					chat::VerifyRequest v_req;
					v_req.set_name(name);
					v_req.set_token(token);
					boost::system::error_code ec;
					while (true)
					{
						timer.expires_after(duration);
						timer.async_wait(yield[ec]);
						if (ec)
						{
							throw tinychat::utility::boost_system_ec_exception(ec);
						}
						chat::VerifyReply v_reply;
						rpc_stub_.async_call(v_req, v_reply, yield[ec]);
						if (ec)
						{
							throw tinychat::utility::boost_system_ec_exception(ec);
						}
						if (!v_reply.ok())
						{
							throw tinychat::utility::worded_exception("server replied not ok");
						}
					}
				}
				catch (const std::exception &e) {
					std::cerr << "heartbeat : " << e.what() << std::endl;
					_exit(EXIT_FAILURE);
				}
			});
		}

		void chat_proc(boost::asio::yield_context yield, boost::asio::io_context &ioc)
		{
			rpc_stub_.rpc_bind<chat::NotifyChatMessageRequest, chat::NotifyChatMessageReply>(
				[this] (const chat::NotifyChatMessageRequest &req, chat::NotifyChatMessageReply &reply)
			{
				this->notify_chat_message_service(req, reply);
			});


			boost::system::error_code ec;

			chat::LoginRequest msg;

			std::cout << "input your name: ";
			std::string name;
			std::getline(std::cin, name);
			msg.set_name(name);

			std::cout << "input your auth key: ";
			std::string auth;
			std::getline(std::cin, auth);
			msg.set_auth(auth);

			chat::LoginReply reply;

			rpc_stub_.async_call(msg, reply, yield[ec]);
			if (ec)
			{
				throw tinychat::utility::boost_system_ec_exception(ec);
			}

			if ( reply.state() != chat::LoginReply::ok )
			{
				switch ( reply.state())
				{
					case (chat::LoginReply::not_registered):
						std::cerr << "name not registered." << std::endl;
						break;
					case (chat::LoginReply::auth_failed):
						std::cerr << "authentication failed." << std::endl;
						break;
					case (chat::LoginReply::duplicate_login):
						std::cerr << "this account is already online. contact the admin."
							<< std::endl;
						break;
					default:
						std::cerr << "error occured during login. quit." << std::endl;
						break;
				}
				return;
			}

			std::cout << "login succeed." << std::endl;
			std::string token = reply.token();
			long long hb_interval = arg_vm["heartbeat"].as<long long>();
			if (hb_interval > 0)
				start_timed_heartbeat(ioc,name,token,std::chrono::seconds(hb_interval));
			std::cout << "your token : " << token << std::endl;
			std::cout << "type to speak" << std::endl;
			std::cout << "------------------" << std::endl;

			while ( true )
			{
				auto context = tinychat::utility::async_stdin_getline(ioc, yield[ec]);
				if (ec)
				{
					throw tinychat::utility::boost_system_ec_exception(ec);
				}
				chat::ChatSendRequest v_req;
				chat::ChatSendReply v_reply;
				v_req.set_name(name);
				v_req.set_token(token);
				v_req.set_text(context);
				rpc_stub_.async_call(v_req, v_reply, yield[ec]);
				if (ec)
				{
					throw tinychat::utility::boost_system_ec_exception(ec);
				}
				if ( v_reply.result() == chat::ChatSendReply::ok )
				{
					std::cout << "message sent" << std::endl;
				}
				else
				{
					std::cout << "message failed to send" << std::endl;
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
		boost::asio::yield_context yield)
{
	boost::system::error_code ec;

	tcp::resolver resolver{ioc};
	ws s{ioc};

	auto const results = resolver.async_resolve(host, port, yield[ec]);
	if (ec)
	{
		throw tinychat::utility::boost_system_ec_exception(ec);
	}

	boost::asio::async_connect(s.next_layer(), results.begin(), results.end(),
			yield[ec]);
	if (ec)
	{
		throw tinychat::utility::boost_system_ec_exception(ec);
	}

	s.async_handshake(host, "/test", yield[ec]);
	if (ec)
	{
		throw tinychat::utility::boost_system_ec_exception(ec);
	}

	s.binary(true);

	auto ses = std::make_shared<rpc_session>(std::move(s));
	boost::asio::spawn(ioc, [&ses] (boost::asio::yield_context yield)
	{
		try
		{
			ses->run(yield);
		}
		catch (const std::exception &e) {
			std::cerr << "error in session : " << e.what() << std::endl;
			_exit(EXIT_FAILURE);
		}
	});
	ses->chat_proc(yield, ioc);
}

int main(int argc, char **argv)
{
	try
	{
		boost::program_options::options_description desc("Options");
		desc.add_options()
			("help", "Print help message")
			("server", boost::program_options::value<std::string>(), "Server address")
			("port", boost::program_options::value<std::string>()->default_value("8000"), "Server port")
			("heartbeat", boost::program_options::value<long long>()->default_value(30),
				"interval to send heartbeat packet, 0 means no heartbeat")
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
			std::cerr << "error : server not specified, use --help for usage" << std::endl;
			return EXIT_FAILURE;
		}

		auto const host = argv[1];
		auto const port = argv[2];

		boost::asio::io_context ioc;

		boost::asio::spawn(ioc, [&ioc](boost::asio::yield_context yield)
		{
			try
			{
				do_session(
					arg_vm["server"].as<std::string>(),
					arg_vm["port"].as<std::string>(),
					ioc,
					yield
				);
			}
			catch (const std::exception &e) {
				std::cerr << "error creating session : " << e.what() << std::endl;
				_exit(EXIT_FAILURE);
			}
		});

		ioc.run();

		return EXIT_SUCCESS;
	}
	catch (const std::exception &e) {
		std::cerr << "error in main process : " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}

