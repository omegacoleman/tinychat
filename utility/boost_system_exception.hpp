#pragma once

#include <exception>
#include "boost/system/error_code.hpp"

// stands for rethrow error_code
// Rethrow boost::system::error_code with message prefix
#define _RT_EC(__prefix, __ec)  \
if (__ec)  \
{  \
	throw tinychat::utility::boost_system_ec_exception(__ec, __prefix, __FILE__, __LINE__);  \
}

namespace tinychat::utility
{
	// This is not 100% right, it requires unix filenames do not present "\"
	// Long as this compiles on windows, this is a condition we must meet..
	const std::string _get_basename(const std::string& path)
	{
		const size_t last_slash_idx = path.find_last_of("\\/");
		if (std::string::npos != last_slash_idx)
		{
			return path.substr(last_slash_idx + 1);
		}
		return path;
	}

	class boost_system_ec_exception : public std::exception
	{
	public:
		[[deprecated("mark with a prefix to show its cause")]] boost_system_ec_exception(boost::system::error_code &ec)
			:message(ec.message()) {}

		boost_system_ec_exception(boost::system::error_code &ec, const std::string &prefix)
			:message(prefix + " : " + ec.message()) {}

		boost_system_ec_exception(boost::system::error_code &ec, const std::string &prefix, const std::string &file, const int line)
			:message(prefix + "[" + _get_basename(file) + "#" + std::to_string(line) + "] : " + ec.message()) {}

		virtual const char* what() const noexcept
		{
			return message.c_str();
		}
		std::string message;
	};

}