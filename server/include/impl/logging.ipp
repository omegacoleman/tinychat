#pragma once

namespace tinychat
{
	namespace logging
	{
		namespace impl
		{
			namespace color_code
			{

				static std::string _reset = "\033[0m";

				static std::string _black = "\033[30m";
				static std::string _red = "\033[31m";
				static std::string _green = "\033[32m";
				static std::string _yellow = "\033[33m";
				static std::string _blue = "\033[34m";
				static std::string _mangenta = "\033[35m";
				static std::string _cyan = "\033[36m";
				static std::string _white = "\033[37m";

				class disabled
				{
				public:
					using color_code_t = const std::string &;

					inline const static std::string reset = "";

					inline const static std::string error_tag = "";
					inline const static std::string warning_tag = "";
					inline const static std::string info_tag = "";

					inline const static std::string error = "";
					inline const static std::string warning = "";
					inline const static std::string info = "";
				};

				class default_scheme
				{
				public:
					using color_code_t = const std::string &;
					
					inline const static std::string reset = _reset;

					inline const static std::string error_tag = _red;
					inline const static std::string warning_tag = _yellow;
					inline const static std::string info_tag = _blue;

					inline const static std::string error = _red;
					inline const static std::string warning = _yellow;
					inline const static std::string info = _white;
				};
			}

			namespace types
			{
				class warning_t {};
				class error_t {};
				class info_t {};
			}

			template <
				typename OStream, 
				typename EndL, 
				typename ColorCode = color_code::disabled
			>
			class base_logger
			{
			public:
				base_logger(OStream &os, EndL endl__)
					:os(os), endl_(endl__)
				{}

				void output(const types::error_t &type, const std::string &tag, const std::string &message)
				{
					colored_print(ColorCode::error_tag, "[" + tag + "]: ");
					colored_print(ColorCode::error, message);
					endl();
				}

				void output(const types::warning_t &type, const std::string &tag, const std::string &message)
				{
					colored_print(ColorCode::warning_tag, "[" + tag + "]: ");
					colored_print(ColorCode::warning, message);
					endl();
				}

				void output(const types::info_t &type, const std::string &tag, const std::string &message)
				{
					colored_print(ColorCode::info_tag, "[" + tag + "]: ");
					colored_print(ColorCode::info, message);
					endl();
				}

			private:

				void colored_print(typename ColorCode::color_code_t color_tag, const std::string &s)
				{
					os << color_tag << s << ColorCode::reset;
				}

				void endl()
				{
					os << endl_;
				}

				OStream &os;
				EndL endl_;
			};
		}
	}
}
