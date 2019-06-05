//
// Created by Maxtorm on 2019/4/10.
//
// mail: martinliao1998@gmail.com

#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/post.hpp>
#include <boost/thread.hpp>

#ifdef TINYCHAT_UNIX

#include <boost/asio/buffer.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/read_until.hpp>

#endif

#include <iostream>

namespace tinychat::utility
{

template <typename CompletionToken>
auto async_stdin_getline(
	boost::asio::io_context &ioc,
	CompletionToken &&token
)
{
	using Result = boost::asio::async_result<CompletionToken, void(boost::system::error_code, std::string)>;
	using Handler = typename Result::completion_handler_type;
	Handler handler(std::forward<CompletionToken>(token));
	Result result(handler);
#if defined(TINYCHAT_WIN)
	boost::thread(
		[&ioc, handler]() mutable
		{
			boost::system::error_code ec;
			std::string context;
			if ( !std::getline(std::cin, context).good())
			{
				ec.assign(GetLastError(), boost::system::system_category());
			}
			boost::asio::post(
				ioc,
				std::bind(handler, ec, context));
		}).detach();
#elif defined(TINYCHAT_UNIX)
	boost::asio::spawn(
		ioc,
		[&ioc, handler](boost::asio::yield_context yield) mutable
		{
			using namespace boost::asio;
			boost::system::error_code ec;
			std::string context;
			posix::stream_descriptor asyncable_stdin(ioc, ::dup(STDIN_FILENO));
			async_read_until(asyncable_stdin, dynamic_buffer(context), '\n', yield[ec]);
			post(ioc,std::bind(handler,ec,context));
		});
#else
#error Platform currently not supported.
#endif
	return result.get();
}

}

