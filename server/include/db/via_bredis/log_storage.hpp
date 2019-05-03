#pragma once

#include <memory>
#include <map>
#include <functional>

#include "boost/asio.hpp"
#include "boost/asio/spawn.hpp"

#include "bredis.hpp"

#include "chatroom.hpp"

#include "boost_system_exception.hpp"
#include "worded_exception.hpp"

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

				template <typename Iterator>
				bool checkin(impl_t::BredisConnection &conn, Iterator it, Iterator end, boost::asio::yield_context yield)
				{
					return detail.checkin(conn, it, end, yield);
				}
			private:
				impl_t detail;
			};
		};
	};
};
