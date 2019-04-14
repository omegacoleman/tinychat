#pragma once

#include <exception>
#include "boost/system/error_code.hpp"

namespace tinychat::utility
{

	class boost_system_ec_exception : public std::exception
	{
	public:
		boost_system_ec_exception(boost::system::error_code &ec)
			:message(ec.message()) {}
		virtual const char* what() const noexcept
		{
			return message.c_str();
		}
		std::string message;
	};

}