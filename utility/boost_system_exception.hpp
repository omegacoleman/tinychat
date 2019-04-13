#pragma once

#include <exception>
#include "boost/system/error_code.hpp"

namespace tinychat::utility
{

	class boost_system_ec_exception : public std::exception
	{
	public:
		boost_system_ec_exception(boost::system::error_code &ec)
			:ec(ec) {}
		virtual const char* what() const noexcept
		{
			return ec.message().c_str();
		}
		boost::system::error_code &ec;
	};

}