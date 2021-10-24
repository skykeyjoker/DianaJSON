#ifndef JSONVALUE_H
#define JSONVALUE_H

#include <variant>

#include "json.h"
#include "jsonerror.h"

namespace DianaJSON {
	// 实现内部类JsonValue
	class JsonValue {
	public:
		// 构造函数
		explicit JsonValue(std::nullptr_t) : _val(nullptr) {}
		explicit JsonValue(bool val) : _val(val) {}
		explicit JsonValue(double val) : _val(val) {}
		explicit JsonValue(const std::string &val) : _val(val) {}
		explicit JsonValue(const Json::_array &val) : _val(val) {}
		explicit JsonValue(const Json::_object &val) : _val(val) {}

	public:
		// 移动构造函数
		explicit JsonValue(std::string &&val) : _val(std::move(val)) {}
		explicit JsonValue(Json::_array &&val) : _val(std::move(val)) {}
		explicit JsonValue(Json::_object &&val) : _val(std::move(val)) {}

	public:
		// 析构函数
		~JsonValue() = default;

	public:
		// Array与Object类的访问接口
		size_t size() const;// 返回数组或对象成员数

		// 数组，支持随机访问
		const Json &operator[](size_t index) const;
		Json &operator[](size_t index);

		// 对象，O(1)遍历寻找（不支持插入）
		const Json &operator[](const std::string &key) const;
		Json &operator[](const std::string &key);

	public:
		// 数据类型判断接口
		JsonValueType getType() const noexcept;

	public:
		// 数据类型转换接口
		std::nullptr_t toNull() const;
		bool toBool() const;
		double toDouble() const;// Number类型实际表现为double类型浮点数
		const std::string &toString() const;
		const Json::_array &toArray() const;
		const Json::_object &toObject() const;

	private:
		std::variant<std::nullptr_t, bool, double, std::string, Json::_array,
					 Json::_object>
				_val;// 使用variant储存多元类型，节省空间
	};
}// namespace DianaJSON

#endif