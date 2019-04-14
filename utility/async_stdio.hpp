//
// Created by Maxtorm on 2019/4/10.
//
// mail: martinliao1998@gmail.com

#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/read_until.hpp>
#ifdef TINYCHAT_UNIX
#include <boost/asio/posix/stream_descriptor.hpp>
#elif defined(TINYCHAT_WIN)
#include <boost/asio/windows/stream_handle.hpp>
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
		spawn(
			ioc,
			[&ioc, handler](boost::asio::yield_context yield) mutable
			{
				using namespace boost::asio;
#if defined(TINYCHAT_WIN)
				windows::stream_handle asyncable_stdin(ioc, GetStdHandle(STD_INPUT_HANDLE));
#elif defined(TINYCHAT_UNIX)
				posix::stream_descriptor asyncable_stdin(ioc, ::dup(STDIN_FILENO));
#endif
				boost::system::error_code ec;
				std::string input;
				async_read_until(asyncable_stdin, dynamic_buffer(input), '\n', yield[ec]);
				post(ioc, std::bind(handler, ec, input));
			});
		return result.get();
	}
	
}

