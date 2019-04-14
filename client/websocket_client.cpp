#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/thread.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <async_stdio.hpp>
#include "chat.pb.h"

#include "tinyrpc/rpc_websocket_service.hpp"

using namespace tinyrpc;

using tcp = boost::asio::ip::tcp;         // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

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
				if ( ec )
					return;
				rpc_stub_.dispatch(buf, ec);
				if ( ec )
					return;
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
			spawn(ioc,
				[duration,name,token,&ioc,this](boost::asio::yield_context yield)
				{
					boost::asio::steady_timer timer(ioc);
					chat::VerifyRequest v_req;
					v_req.set_name(name);
					v_req.set_token(token);
					boost::system::error_code ec;
					boost::beast::websocket::close_reason reason;
					try
					{
						while(true)
						{
							timer.expires_after(duration);
							timer.async_wait(yield);
							chat::VerifyReply v_reply;
							rpc_stub_.async_call(v_req, v_reply, yield);
							if (!v_reply.ok())
							{
								reason.reason = "connection interrupted.";
								break;
							}
						}
					}
					catch (boost::system::system_error &error)
					{
						reason.reason = std::string("System error: ")+error.what();
					}
					ws_.close(reason);
				});
		}

		void chat_proc(boost::asio::yield_context yield, boost::asio::io_context &ioc)
		{
			rpc_stub_.rpc_bind<chat::NotifyChatMessageRequest, chat::NotifyChatMessageReply>(
					std::bind(&rpc_session::notify_chat_message_service, this,
						std::placeholders::_1, std::placeholders::_2));


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
			if ( ec )
			{
				std::cout << "error: " << ec.message() << std::endl;
				return;
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
			// start_timed_heartbeat(ioc,name,token,std::chrono::seconds(30));
			std::cout << "your token : " << token << std::endl;
			std::cout << "type to speak" << std::endl;
			std::cout << "------------------" << std::endl;

			while ( true )
			{
				auto context = tinychat::utility::async_stdin_getline(ioc,yield[ec]);
				{
					chat::ChatSendRequest v_req;
					chat::ChatSendReply v_reply;
					v_req.set_name(name);
					v_req.set_token(token);
					v_req.set_text(context);
					rpc_stub_.async_call(v_req, v_reply, yield[ec]);
					if ( ec )
					{
						std::cerr << "error: " << ec.message() << std::endl;
						return;
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
		}

	private:
		ws ws_;
		rpc_websocket_service<ws> rpc_stub_;
};


void fail(boost::system::error_code ec, char const *what)
{
	std::cerr << what << ": " << ec.message() << "\n";
}

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
	if ( ec )
		return fail(ec, "resolve");

	boost::asio::async_connect(s.next_layer(), results.begin(), results.end(),
			yield[ec]);
	if ( ec )
		return fail(ec, "connect");

	s.async_handshake(host, "/test", yield[ec]);
	if ( ec )
		return fail(ec, "handshake");

	s.binary(true);

	auto ses = std::make_shared<rpc_session>(std::move(s));
	boost::asio::spawn(ioc,
			std::bind(&rpc_session::run, ses, std::placeholders::_1));
	ses->chat_proc(yield, ioc);
}

int main(int argc, char **argv)
{
	if ( argc != 3 )
	{
		std::cerr <<
			"Usage: websocket-client <host> <port>\n" <<
			"Example:\n" <<
			"	websocket-client 127.0.0.1 8000\n";
		return EXIT_FAILURE;
	}

	auto const host = argv[1];
	auto const port = argv[2];

	boost::asio::io_context ioc;

	boost::asio::spawn(ioc, std::bind(
				&do_session,
				std::string(host),
				std::string(port),
				std::ref(ioc),
				std::placeholders::_1));

	ioc.run();

	return EXIT_SUCCESS;
}

