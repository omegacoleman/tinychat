#pragma once

#include <memory>
#include <map>
#include <functional>
#include <vector>

#include "chatroom.hpp"

namespace chatroom
{
	namespace db
	{
		namespace dummy
		{
			template <typename Generator, typename Handle>
			void dummy_db_checkin(Generator next, Handle callback)
			{
				std::cout << "dummy_client.hpp : Pretending to checkin these to db : " << std::endl;
				std::cout << "-----------------------------------" << std::endl;
				for (auto it = next(); it != nullptr; it = next())
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
				{"jujube", "4"}
			};

			bool auth_func(const std::string &name, const std::string &auth)
			{
				return (dummy_user_auth.at(name) == auth);
			}

			bool is_banned(const std::string &name)
			{
				return (name == "jujube");
			}
		};
	};
};
