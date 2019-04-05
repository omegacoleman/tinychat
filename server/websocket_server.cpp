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


using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

using ws = websocket::stream<tcp::socket>;

class rpc_session;

std::unique_ptr<chatroom::Room<rpc_session> > room;

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
        } catch(chatroom::UserNotFoundException e) {
            reply.set_state(chat::LoginReply::not_registered);
        } catch(chatroom::AuthenticateFailedException e) {
            reply.set_state(chat::LoginReply::auth_failed);
        } catch(chatroom::DuplicateLoginException e) {
            reply.set_state(chat::LoginReply::duplicate_login);
        } catch(std::exception e) {
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
    boost::asio::io_context& ioc,
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
                acceptor.get_executor().context(),
                std::bind(
                    &do_session,
                    std::move(socket),
                    std::placeholders::_1));
    }
}

int main(int argc, char* argv[])
{
    room = chatroom::Room<rpc_session>::load_room();
    if (argc != 3)
    {
        std::cerr <<
            "Usage: websocket-server <address> <port>\n" <<
            "Example:\n" <<
            "    websocket-server 0.0.0.0 8000\n";
        return EXIT_FAILURE;
    }
    auto const address = boost::asio::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));

    boost::asio::io_context ioc;

    boost::asio::spawn(ioc,
        std::bind(
            &do_listen,
            std::ref(ioc),
            tcp::endpoint{address, port},
            std::placeholders::_1));

    ioc.run();

    return EXIT_SUCCESS;
}
