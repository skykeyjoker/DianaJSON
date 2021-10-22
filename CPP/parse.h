#ifndef PARSE_H
#define PARSE_H

#include "json.h"
//#include "jsonerror.h"

namespace DianaJSON {
	inline constexpr bool is1to9(char ch) { return ch >= '1' && ch <= '9'; }
	inline constexpr bool is0to9(char ch) { return ch >= '0' && ch <= '9'; }

	class Parser {
	public:
		// 构造函数
		explicit Parser(const char* cstr) noexcept : _start(cstr),
													 _curr(cstr) {}
		explicit Parser(const std::string& context) noexcept : _start(context.c_str()),
															   _curr(context.c_str()) {}

	public:
		// 禁止拷贝
		Parser(const Parser&) = delete;
		Parser& operator=(const Parser&) = delete;

	public:
		// 唯一对外开放的解析结果获取接口
		Json parse();

	private:
		// 内部解析方法
		Json parseValue();
		Json parseLiteral(const std::string& literal);
		Json parseArray();
		Json parseObject();
		Json parseString();
		Json parseNumber();

	private:
		void parseWhitespace() noexcept;
		std::string parseRawString();
		unsigned parse4hex();
		std::string encodeUTF8(unsigned u) noexcept;

	private:
		const char* _start;
		const char* _curr;
	};
}// namespace DianaJSON


#endif