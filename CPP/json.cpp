#include "json.h"
//#include "parse.h"
#include <cstdio>

#include "jsonvalue.h"

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
	Json *Json::operator=(Json &&) noexcept = default;

	// 数据接口
	JsonValueType Json::getType() const noexcept {
		return _value->getType();
	}
	bool Json::isNull() const {
		return _value->getType() == JsonValueType::Null;
	}
	bool Json::isBoolean() const {
		return _value->getType() == JsonValueType::Bool;
	}
	bool Json::isNumber() const {
		return _value->getType() == JsonValueType::Number;
	}
	bool Json::isString() const {
		return _value->getType() == JsonValueType::String;
	}
	bool Json::isArray() const {
		return _value->getType() == JsonValueType::Array;
	}
	bool Json::isObject() const {
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

}// namespace DianaJSON