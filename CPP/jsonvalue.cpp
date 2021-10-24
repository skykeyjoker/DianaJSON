#include "jsonvalue.h"

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

	std::nullptr_t JsonValue::toNull() const {
		try {
			return std::get<std::nullptr_t>(_val);
		} catch (const std::bad_variant_access &) {
			throw JsonException("not a null");
		}
	}

	bool JsonValue::toBool() const {
		try {
			return std::get<bool>(_val);
		} catch (const std::bad_variant_access &) {
			throw JsonException("not a bool");
		}
	}

	double JsonValue::toDouble() const {
		try {
			return std::get<double>(_val);
		} catch (const std::bad_variant_access &) {
			throw JsonException("not a double");
		}
	}

	const std::string &JsonValue::toString() const {
		try {
			return std::get<std::string>(_val);
		} catch (std::bad_variant_access &) {
			throw JsonException("not a string");
		}
	}

	const Json::_array &JsonValue::toArray() const {
		try {
			return std::get<Json::_array>(_val);
		} catch (std::bad_variant_access &) {
			throw JsonException("not a array");
		}
	}

	const Json::_object &JsonValue::toObject() const {
		try {
			return std::get<Json::_object>(_val);
		} catch (std::bad_variant_access &) {
			throw JsonException("not a object");
		}
	}

	size_t JsonValue::size() const {
		if (std::holds_alternative<Json::_array>(_val))
			return std::get<Json::_array>(_val).size();
		else if (std::holds_alternative<Json::_object>(_val))
			return std::get<Json::_object>(_val).size();
		else {
			throw JsonException("not a array or object");
		}
	}

	const Json &JsonValue::operator[](size_t index) const {
		if (std::holds_alternative<Json::_array>(_val)) {
			return std::get<Json::_array>(_val)[index];
		} else {
			throw JsonException("not a array");
		}
	}

	Json &JsonValue::operator[](size_t index) {
		return const_cast<Json &>(static_cast<const JsonValue &>(*this)[index]);
	}

	const Json &JsonValue::operator[](const std::string &key) const {
		if (std::holds_alternative<Json::_object>(_val)) {
			return std::get<Json::_object>(_val).at(key);
		} else {
			throw JsonException("not a object");
		}
	}

	Json &JsonValue::operator[](const std::string &key) {
		return const_cast<Json &>(static_cast<const JsonValue &>(*this)[key]);
	}

}// namespace DianaJSON