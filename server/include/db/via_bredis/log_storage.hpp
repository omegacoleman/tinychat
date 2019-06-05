#pragma once

#include <memory>
#include <map>
#include <functional>

#include "boost/asio.hpp"
#include "boost/asio/spawn.hpp"

#include "bredis.hpp"

#include "chatroom.hpp"

#include "utility/boost_system_exception.hpp"
#include "utility/worded_exception.hpp"

#include "impl/log_storage.ipp"

namespace chatroom
{
	namespace db
	{
		namespace via_bredis
		{
			class log_storage
			{
			public:
				typedef typename impl::base_log_storage<
					boost::asio::ip::tcp::socket,
					boost::asio::streambuf
				> impl_t;

				template <typename Generator>
				bool checkin(impl_t::BredisConnection &conn, Generator next, boost::asio::yield_context yield)
				{
					return detail.checkin(conn, next, yield);
				}
			private:
				impl_t detail;
			};
		};
	};
};
