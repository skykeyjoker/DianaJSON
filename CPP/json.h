#ifndef JSON_H
#define JSON_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace DianaJSON {
	// 声明JSON数据类型
	enum class JsonValueType {
		Null,
		Bool,
		Number,
		String,
		Array,
		Object
	};

	// 为JsonValue内部类前向声明（std::unique_ptr）
	class JsonValue;

	class Json {
	public:
		// 类型重名
		using _array = std::vector<Json>;
		using _object = std::unordered_map<std::string, Json>;

	public:
		// 构造函数
		Json() : Json(nullptr) {}
		Json(std::nullptr_t);
		Json(bool);
		Json(int n) : Json(1.0 * n) {}
		Json(double);
		Json(const char *str) : Json(std::string(str)) {}
		Json(const std::string &);
		Json(std::string &&);
		Json(const Json::_array &);
		Json(Json::_array &&);
		Json(const Json::_object &);
		Json(Json::_object &&);

		Json(void *) = delete;

	public:
		// 析构函数
		~Json() = default;

	public:
		// 拷贝
		Json(const Json &);                    // 深拷贝
		Json &operator=(const Json &) noexcept;// 拷贝，交换

	public:
		// 移动
		Json(Json &&) noexcept;
		Json *operator=(Json &&) noexcept;

	public:
		// 数据类型判断接口
		JsonValueType
		getType() const noexcept;
		bool isNull() const;
		bool isBoolean() const;
		bool isNumber() const;
		bool isString() const;
		bool isArray() const;
		bool isObject() const;

	public:
		// 数据类型转换接口
		bool toBool() const;
		double toDouble() const;// Number类型实际表现为double类型浮点数
		const std::string &toString() const;
		const _array &toArray() const;
		const _object &toObject() const;

	public:
		// 解析与生成器接口
		static Json parse(const std::string &context, std::string &errorText) noexcept;// 解析
		std::string serialize() const noexcept;                                        // 生成器

	private:
		std::unique_ptr<JsonValue>
				_value;// PIMPL
	};
}// namespace DianaJSON

#endif