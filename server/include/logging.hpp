#pragma once

#include <iostream>
#include <sstream>
#include "impl/logging.ipp"

namespace tinychat
{
	namespace logging
	{
		class logger
		{
		public:
			logger(const logger &other) = delete;
			void operator=(const logger &other) = delete;

			template <typename LogType>
			class collector
			{
			public:
				collector(const std::string &tag, logger &binded_logger)
					:tag(tag), binded_logger(binded_logger)
				{}

				template <typename T>
				collector & operator<<(T && in)
				{
					ss << std::forward<T>(in);
					return *this;
				}

				~collector()
				{
					binded_logger.output(type, tag, ss.str());
				}
			private:
				logger &binded_logger;
				LogType type{};
				std::string tag;
				std::ostringstream ss;
			};

			static logger &instance()
			{
				static logger only;
				return only;
			}

			void output(const impl::types::error_t &type, const std::string &tag, const std::string &message)
			{
				detail.output(type, tag, message);
			}

			void output(const impl::types::warning_t &type, const std::string &tag, const std::string &message)
			{
				detail.output(type, tag, message);
			}

			void output(const impl::types::info_t &type, const std::string &tag, const std::string &message)
			{
				detail.output(type, tag, message);
			}

			collector<impl::types::error_t> error(const std::string &tag)
			{
				return { tag, *this };
			}

			collector<impl::types::warning_t> warning(const std::string &tag)
			{
				return { tag, *this };
			}

			collector<impl::types::info_t> info(const std::string &tag)
			{
				return { tag, *this };
			}

		private:
			logger() {}

			impl::base_logger<
				std::ostream, 
				char, 
				impl::color_code::default_scheme
			> detail{std::cerr, '\n'};
		};
	}
}
