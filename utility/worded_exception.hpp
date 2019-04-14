#pragma once

#include <exception>

namespace tinychat::utility
{

	class worded_exception : public std::exception
	{
	public:
		worded_exception(const std::string &message)
			:message(message) {}
		virtual const char* what() const noexcept
		{
			return message.c_str();
		}
		std::string message;
	};

}
