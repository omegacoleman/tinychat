#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "chat.pb.h"

#include "chatroom.hpp"

#include "tinyrpc/rpc_websocket_service.hpp"
using namespace tinyrpc;


using tcp = boost::asio::ip::tcp;	 // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

using ws = websocket::stream<tcp::socket>;

boost::asio::io_context ioc;

class rpc_session;

void dummy_db_checkin(chatroom::LogIterator start, chatroom::LogIterator end, std::function<void ()> callback)
{
	std::cout << "Pretending to checkin these to db : " << std::endl;
	std::cout << "-----------------------------------" << std::endl;
	for(; start != end; ++start)
	{
		std::cout << start->sender << " : " << start->text << std::endl;
	}
	std::cout << "-----------------------------------" << std::endl;
	callback();
}

std::unique_ptr<chatroom::Room<rpc_session> > room;
std::unique_ptr<chatroom::ChatLog<> > chatlog;

class VendorBoostSystemException : public std::exception
{
public:
	VendorBoostSystemException(boost::system::error_code &ec)
	:ec(ec) {}
	virtual const char* what() const noexcept
	{
		return ec.message().c_str();
	}
	boost::system::error_code &ec;
};

class rpc_session : public std::enable_shared_from_this<rpc_session>
{
public:
	rpc_session(ws&& s)
	: ws_(std::move(s))
	, rpc_stub_(ws_)
	{}

	~rpc_session()
	{
		std::cout << "~session\n";
	}

	void run(boost::asio::yield_context yield)
	{
		rpc_stub_.rpc_bind<chat::LoginRequest, chat::LoginReply>(
			std::bind(&rpc_session::login_service, this,
				std::placeholders::_1, std::placeholders::_2));
		rpc_stub_.rpc_bind<chat::VerifyRequest, chat::VerifyReply>(
			std::bind(&rpc_session::verify_service, this,
				std::placeholders::_1, std::placeholders::_2));
		rpc_stub_.rpc_bind<chat::ChatSendRequest, chat::ChatSendReply>(
			std::bind(&rpc_session::chat_send_service, this,
				std::placeholders::_1, std::placeholders::_2));

		boost::beast::multi_buffer buf;
		boost::system::error_code ec;

		while (true)
		{
			auto bytes = ws_.async_read(buf, yield[ec]);
			if (ec)
				return;
			rpc_stub_.dispatch(buf, ec);
			if (ec)
				return;
			buf.consume(bytes);
		}
	}

	void login_service(const chat::LoginRequest& req, chat::LoginReply& reply)
	{
		try
		{
			std::cout << req.name() << " wants to login. " << std::endl;
			std::string token = room->login(req.name(), req.auth(), *this);
			reply.set_state(chat::LoginReply::ok);
			reply.set_token(token);
			std::cout << req.name() << " logged in " << std::endl;
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
			throw VendorBoostSystemException(ec);
		}
	}

	void deliver(std::shared_ptr<chatroom::Message> message)
	{
		boost::asio::spawn(ioc,
			std::bind(
				&rpc_session::deliver_proc,
				this, 
				message,
				std::placeholders::_1));
	}

private:
	ws ws_;
	rpc_websocket_service<ws> rpc_stub_;
};


void fail(boost::system::error_code ec, char const* what)
{
	std::cerr << what << ": " << ec.message() << "\n";
}

void do_session(tcp::socket& socket, boost::asio::yield_context yield)
{
	boost::system::error_code ec;

	ws s{std::move(socket)};

	s.async_accept(yield[ec]);
	if(ec)
		return fail(ec, "accept");

	s.binary(true);

	auto ses = std::make_shared<rpc_session>(std::move(s));
	ses->run(yield);
}

void do_listen(
	tcp::endpoint endpoint,
	boost::asio::yield_context yield)
{
	boost::system::error_code ec;

	tcp::acceptor acceptor(ioc);
	acceptor.open(endpoint.protocol(), ec);
	if(ec)
		return fail(ec, "open");

	acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
	if(ec)
		return fail(ec, "set_option");

	acceptor.bind(endpoint, ec);
	if(ec)
		return fail(ec, "bind");

	acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
	if(ec)
		return fail(ec, "listen");

	for(;;)
	{
		tcp::socket socket(ioc);
		acceptor.async_accept(socket, yield[ec]);
		if(ec)
			fail(ec, "accept");
		else
			boost::asio::spawn(
				ioc,
				std::bind(
					&do_session,
					std::move(socket),
					std::placeholders::_1));
	}
}

int main(int argc, char* argv[])
{
	room = chatroom::Room<rpc_session>::load_room();
	chatlog = std::make_unique<chatroom::ChatLog<> >(30, 10, 14);
	std::function f{dummy_db_checkin};
	chatlog->auto_checkin(f);

	if (argc != 3)
	{
		std::cerr <<
		"Usage: websocket-server <address> <port>\n" <<
		"Example:\n" <<
		"   websocket-server 0.0.0.0 8000\n";
		return EXIT_FAILURE;
	}
	auto const address = boost::asio::ip::make_address(argv[1]);
	auto const port = static_cast<unsigned short>(std::atoi(argv[2]));

	boost::asio::spawn(ioc,
		std::bind(
			&do_listen,
			tcp::endpoint{address, port},
			std::placeholders::_1));

	ioc.run();

	return EXIT_SUCCESS;
}
