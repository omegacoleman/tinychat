#pragma once

#include <memory>
#include <map>
#include <functional>

#include "chatroom.hpp"

namespace chatroom
{
	namespace db
	{
		namespace dummy
		{
			void dummy_db_checkin(chatroom::ChatLog::LogIterator it, chatroom::ChatLog::LogIterator end, std::function<void()> callback)
			{
				std::cout << "dummy_client.hpp : Pretending to checkin these to db : " << std::endl;
				std::cout << "-----------------------------------" << std::endl;
				for (; it != end; ++it)
				{
					std::cout << it->id << std::endl;
				}
				std::cout << "-----------------------------------" << std::endl;
				callback();
			}

			std::map<std::string, std::string> dummy_user_auth{
				{"banana", "1"},
				{"papaya", "2"},
				{"cocoa", "3"},
			};
		};
	};
};
