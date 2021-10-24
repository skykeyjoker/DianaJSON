#include "jsonvalue.h"
//#include "jsonerror.h"

namespace DianaJSON {
	JsonValueType JsonValue::getType() const noexcept {
		if (std::holds_alternative<std::nullptr_t>(_val))
			return JsonValueType::Null;
		else if (std::holds_alternative<bool>(_val))
			return JsonValueType::Bool;
		else if (std::holds_alternative<double>(_val))
			return JsonValueType::Number;
		else if (std::holds_alternative<std::string>(_val))
			return JsonValueType::String;
		else if (std::holds_alternative<Json::_array>(_val))
			return JsonValueType::Array;
		else if (std::holds_alternative<Json::_object>(_val))
			return JsonValueType::Object;
	}

	std::nullptr_t JsonValue::toNull() const { return std::get<std::nullptr_t>(_val); }

	bool JsonValue::toBool() const { return std::get<bool>(_val); }

	double JsonValue::toDouble() const { return std::get<double>(_val); }

	const std::string &JsonValue::toString() const {
		return std::get<std::string>(_val);
	}

	const Json::_array &JsonValue::toArray() const {
		return std::get<Json::_array>(_val);
	}

	const Json::_object &JsonValue::toObject() const {
		return std::get<Json::_object>(_val);
	}

	size_t JsonValue::size() const {
		if (std::holds_alternative<Json::_array>(_val))
			return std::get<Json::_array>(_val).size();
		else if (std::holds_alternative<Json::_object>(_val))
			return std::get<Json::_object>(_val).size();
		else {
			// TODO 非数组或者对象类型报错
		}
	}

	const Json &JsonValue::operator[](size_t index) const {
		if (std::holds_alternative<Json::_array>(_val)) {
			return std::get<Json::_array>(_val)[index];
		} else {
			// TODO 非数组类型报错
		}
	}

	Json &JsonValue::operator[](size_t index) {
		return const_cast<Json &>(static_cast<const JsonValue &>(*this)[index]);
	}

	const Json &JsonValue::operator[](const std::string &key) const {
		if (std::holds_alternative<Json::_object>(_val)) {
			return std::get<Json::_object>(_val).at(key);
		} else {
			// TODO 非对象类型报错
		}
	}

	Json &JsonValue::operator[](const std::string &key) {
		return const_cast<Json &>(static_cast<const JsonValue &>(*this)[key]);
	}

}// namespace DianaJSON