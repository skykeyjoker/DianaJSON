#include "parse.h"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace DianaJSON {

	// 跳过所有白空格
	void Parser::parseWhitespace() noexcept {
		while (*_curr == ' ' || *_curr == '\t' || *_curr == '\r' || *_curr == '\n') {
			++_curr;
		}
		_start = _curr;
	}

	unsigned Parser::parse4hex() {
		unsigned u = 0;
		for (int i = 0; i != 4; ++i) {
			auto ch = static_cast<unsigned>(toupper(*++_curr));
			u <<= 4;
			if (ch >= '0' && ch <= '9') {
				u |= (ch - '0');
			} else if (ch >= 'A' && ch <= 'F') {
				u |= ch - 'A' + 10;
			} else {
				// TODO 非法UNICODE HEX
			}
		}
		return u;
	}

	std::string Parser::encodeUTF8(unsigned int u) noexcept {
		std::string utf8;
		if (u <= 0x7F) {// 0111,1111
			utf8.push_back(static_cast<char>(u & 0xff));
		} else if (u <= 0x7FF) {
			utf8.push_back(static_cast<char>(0xc0 | ((u >> 6) & 0xff)));
			utf8.push_back(static_cast<char>(0x80 | (u & 0x3f)));
		} else if (u <= 0xFFFF) {
			utf8.push_back(static_cast<char>(0xe0 | ((u >> 12) & 0xff)));
			utf8.push_back(static_cast<char>(0x80 | ((u >> 6) & 0x3f)));
			utf8.push_back(static_cast<char>(0x80 | (u & 0x3f)));
		} else {
			assert(u <= 0x10FFFF);
			utf8.push_back(static_cast<char>(0xf0 | ((u >> 18) & 0xff)));
			utf8.push_back(static_cast<char>(0x80 | ((u >> 12) & 0x3f)));
			utf8.push_back(static_cast<char>(0x80 | ((u >> 6) & 0x3f)));
			utf8.push_back(static_cast<char>(0x80 | (u & 0x3f)));
		}
		return utf8;
	}

	std::string Parser::parseRawString() {
		std::string str;
		while (true) {
			switch (*++_curr) {
				case '\"':// 到达字符串末尾
					_start = ++_curr;
					return str;
				case '\0':
					// TODO 缺少末尾引号
				default:
					if (static_cast<unsigned char>(*_curr) < 0x20) {
						// TODO 非法字符
					}
					str.push_back(*_curr);
					break;
				case '\\':// 转义字符
					switch (*++_curr) {
						case '\"':
							str.push_back('\"');
							break;
						case '\\':
							str.push_back('\\');
							break;
						case '/':
							str.push_back('/');
							break;
						case 'b':
							str.push_back('\b');
							break;
						case 'f':
							str.push_back('\f');
							break;
						case 'n':
							str.push_back('\n');
							break;
						case 't':
							str.push_back('\t');
							break;
						case 'r':
							str.push_back('\r');
							break;
						case 'u': {
							unsigned u1 = parse4hex();
							if (u1 >= 0xd800 && u1 <= 0xdbff) {// 高代理区
								if (*++_curr != '\\') {
									// TODO 错误代理区
								}
								if (*++_curr != 'u') {
									// TODO 错误代理区
								}
								unsigned u2 = parse4hex();// 低代理区
								if (u2 < 0xdc00 || u2 > 0xdfff) {
									// TODO 错误代理区
								}
								u1 = (((u1 - 0xd800) << 10) | (u2 - 0xdc00)) + 0x10000;
							}
							str += encodeUTF8(u1);
						} break;
						default: {
							// TODO 错误字符串文本
						}
					}
					break;
			}
		}
	}

	Json Parser::parseValue() {
		// 值类型分发
		switch (*_curr) {
			case 'n':
				return parseLiteral("null");
			case 'f':
				return parseLiteral("false");
			case 't':
				return parseLiteral("true");
			case '\"':
				return parseString();
			case '[':
				return parseArray();
			case '{':
				return parseObject();
			case '\0':
				// TODO 非法JSON文本
				break;
			default:
				return parseNumber();
		}
	}

	Json Parser::parseLiteral(const std::string &literal) {
		// 解析null，false，true
		if (strncmp(_curr, literal.c_str(), literal.size()) != 0) {
			// TODO 非null，false或者true
		}
		_curr += literal.size();
		_start = _curr;
		switch (literal[0]) {
			case 't':
				return Json(true);
			case 'f':
				return Json(false);
			default:
				return Json(nullptr);
		}
	}

	Json Parser::parseNumber() {
		if (*_curr == '-') ++_curr;// 负数
		if (*_curr == '0')         // 前导零
			++_curr;
		else {
			if (!is1to9(*_curr)) {
				// TODO 非法数字文本
			}
			while (is0to9(*++_curr))
				;// 通过所有合法数字
		}
		if (*_curr == '.') {
			// 小数点后必须是数字
			if (!is0to9(*++_curr)) {
				// TODO 非法数字文本
			}
			while (is0to9(*++_curr))
				;
		}
		if (toupper(*_curr) == 'E') {
			++_curr;
			if (*_curr == '-' || *_curr == '+') ++_curr;
			if (!is0to9(*_curr)) {
				// TODO 非法数字文本
			}
			while (is0to9(*++_curr))
				;
		}
		// 经过以上步骤后便可确认该数字合法
		double val = strtod(_start, nullptr);
		if (fabs(val) == HUGE_VAL) {
			// TODO 数字过大
		}
		_start = _curr;
		return Json(val);
	}

	Json Parser::parseString() {
		return Json(parseRawString());
	}

	Json Parser::parseArray() {
		Json::_array arr;
		++_curr;// 跳过'['
		parseWhitespace();
		if (*_curr == ']') {
			_start = ++_curr;
			return Json(arr);
		}
		while (true) {
			parseWhitespace();
			arr.push_back(parseValue());
			parseWhitespace();
			if (*_curr == ',')
				_curr++;
			else if (*_curr == ']') {
				_start = ++_curr;
				return Json(arr);
			} else {
				// TODO 错误，缺少逗号或者方括号
			}
		}
	}

	Json Parser::parseObject() {
		Json::_object obj;
		++_curr;// 跳过'{'
		parseWhitespace();
		if (*_curr == '}') {
			_start = ++_curr;
			return Json(obj);
		}
		while (true) {
			parseWhitespace();
			if (*_curr != '"') {
				// TODO 错误，缺失key
			}
			std::string key = parseRawString();
			parseWhitespace();
			if (*_curr++ != ':') {
				// TODO 错误，缺少冒号
			}
			parseWhitespace();
			Json val = parseValue();
			obj.insert({key, val});
			parseWhitespace();
			if (*_curr == ',')
				_curr++;
			else if (*_curr == '}') {
				_start = ++_curr;
				return Json(obj);
			} else {
				// TODO 错误，缺少逗号或者花括号
			}
		}
	}

	Json Parser::parse() {
		// Json-text = ws value ws
		parseWhitespace();
		Json json = parseValue();
		parseWhitespace();
		if (*_curr) {
			// TODO 仍剩余部分字符未处理
		}
		return json;
	}

}// namespace DianaJSON