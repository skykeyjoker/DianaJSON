#include "json.h"

#include <cstdio>

#include "jsonvalue.h"
#include "parse.h"

namespace DianaJSON {
	// 构造函数
	Json::Json(std::nullptr_t) : _value(std::make_unique<JsonValue>(nullptr)) {
	}
	Json::Json(bool val) : _value(std::make_unique<JsonValue>(val)) {
	}
	Json::Json(double val) : _value(std::make_unique<JsonValue>(val)) {
	}
	Json::Json(const std::string &val) : _value(std::make_unique<JsonValue>(val)) {
	}
	Json::Json(std::string &&val) : _value(std::make_unique<JsonValue>(std::move(val))) {
	}
	Json::Json(const Json::_array &val) : _value(std::make_unique<JsonValue>(val)) {
	}
	Json::Json(Json::_array &&val) : _value(std::make_unique<JsonValue>(std::move(val))) {
	}
	Json::Json(const Json::_object &val) : _value(std::make_unique<JsonValue>(val)) {
	}
	Json::Json(Json::_object &&val) : _value(std::make_unique<JsonValue>(std::move(val))) {
	}

	Json::~Json() = default;

	Json::Json(const Json &rhs) {
		switch (rhs.getType()) {
			case JsonValueType::Null: {
				_value = std::make_unique<JsonValue>(nullptr);
				break;
			}
			case JsonValueType::Bool: {
				_value = std::make_unique<JsonValue>(rhs.toBool());
				break;
			}
			case JsonValueType::Number: {
				_value = std::make_unique<JsonValue>(rhs.toDouble());
				break;
			}
			case JsonValueType::String: {
				_value = std::make_unique<JsonValue>(rhs.toString());
				break;
			}
			case JsonValueType::Array: {
				_value = std::make_unique<JsonValue>(rhs.toArray());
				break;
			}
			case JsonValueType::Object: {
				_value = std::make_unique<JsonValue>(rhs.toObject());
				break;
			}
			default: {
				break;
			}
		}
	}

	Json &Json::operator=(const Json &rhs) noexcept {
		Json temp(rhs);// 深拷贝
		std::swap(_value, temp._value);
		return *this;
	}

	Json::Json(Json &&) noexcept = default;
	Json &Json::operator=(Json &&) noexcept = default;


	// 数据接口
	JsonValueType Json::getType() const noexcept {
		return _value->getType();
	}
	bool Json::isNull() const noexcept {
		return _value->getType() == JsonValueType::Null;
	}
	bool Json::isBoolean() const noexcept {
		return _value->getType() == JsonValueType::Bool;
	}
	bool Json::isNumber() const noexcept {
		return _value->getType() == JsonValueType::Number;
	}
	bool Json::isString() const noexcept {
		return _value->getType() == JsonValueType::String;
	}
	bool Json::isArray() const noexcept {
		return _value->getType() == JsonValueType::Array;
	}
	bool Json::isObject() const noexcept {
		return _value->getType() == JsonValueType::Object;
	}

	bool Json::toBool() const {
		return _value->toBool();
	}
	double Json::toDouble() const {
		return _value->toDouble();
	}
	const std::string &Json::toString() const {
		return _value->toString();
	}
	const Json::_array &Json::toArray() const {
		return _value->toArray();
	}
	const Json::_object &Json::toObject() const {
		return _value->toObject();
	}

	size_t Json::size() const {
		return _value->size();
	}
	Json &Json::operator[](size_t pos) {
		return _value->operator[](pos);
	}
	const Json &Json::operator[](size_t pos) const {
		return _value->operator[](pos);
	}
	Json &Json::operator[](const std::string &key) {
		return _value->operator[](key);
	}
	const Json &Json::operator[](const std::string &key) const {
		return _value->operator[](key);
	}

	Json Json::parse(const std::string &context, std::string &errorText) noexcept {
		Parser p(context);

		// TODO 错误返回
		return p.parse();
	}

	std::string Json::serialize() const noexcept {
		switch (_value->getType()) {
			case JsonValueType::Null:
				return "null";
			case JsonValueType::Bool:
				return _value->toBool() ? "true" : "false";
			case JsonValueType::Number:
				char buf[32];
				snprintf(
						buf, sizeof(buf), "%.17g",
						_value->toDouble());
				return std::string(buf);
			case JsonValueType::String:
				return serializeString();
			case JsonValueType::Array:
				return serializeArray();
			default:
				return serializeObject();
		}
	}
	std::string Json::serializeString() const noexcept {
		std::string res = "\"";
		for (auto e : _value->toString()) {
			switch (e) {
				case '\"':
					res += "\\\"";
					break;
				case '\\':
					res += "\\\\";
					break;
				case '\b':
					res += "\\b";
					break;
				case '\f':
					res += "\\f";
					break;
				case '\n':
					res += "\\n";
					break;
				case '\r':
					res += "\\r";
					break;
				case '\t':
					res += "\\t";
					break;
				default:
					if (static_cast<unsigned char>(e) < 0x20) {
						char buf[7];
						sprintf(buf, "\\u%04X", e);
						res += buf;
					} else
						res += e;
			}
		}
		return res + '"';
	}
	std::string Json::serializeArray() const noexcept {
		std::string res = "[ ";
		for (size_t i = 0; i != _value->size(); ++i) {
			if (i > 0) {
				res += ", ";
			}
			res += (*this)[i].serialize();
		}
		return res + " ]";
	}
	std::string Json::serializeObject() const noexcept {
		std::string res = "{ ";
		bool first = true;
		for (auto &&p : _value->toObject()) {
			if (first) {
				first = false;
			} else {
				res += ", ";
			}
			res += "\"" + p.first + "\"";
			res += ": ";
			res += p.second.serialize();
		}
		return res + " }";
	}

	bool operator==(const Json &lhs, const Json &rhs) {
		if (lhs.getType() != rhs.getType()) {
			return false;
		}
		switch (lhs.getType()) {
			case JsonValueType::Null: {
				return true;
			}
			case JsonValueType::Bool: {
				return lhs.toBool() == rhs.toBool();
			}
			case JsonValueType::Number: {
				return lhs.toDouble() == rhs.toDouble();
			}
			case JsonValueType::String: {
				return lhs.toString() == rhs.toString();
			}
			case JsonValueType::Array: {
				return lhs.toArray() == rhs.toArray();
			}
			case JsonValueType::Object: {
				return lhs.toObject() == rhs.toObject();
			}
			default: {
				return false;
			}
		}
	}

}// namespace DianaJSON