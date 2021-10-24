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

	class Json final {
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
		// 对象类构造函数
		// map-like objects (std::map, std::unordered_map, ...)
		template<
				class M,
				typename std::enable_if<
						std::is_constructible<
								std::string,
								decltype(std::declval<M>().begin()->first)>::value &&
								std::is_constructible<
										Json, decltype(std::declval<M>().begin()->second)>::value,
						int>::type = 0>
		Json(const M &m) : Json(_object(m.begin(), m.end())) {}
		// vector-like objects (std::list, std::vector, std::set, ...)
		template<class V,
				 typename std::enable_if<
						 std::is_constructible<
								 Json, decltype(*std::declval<V>().begin())>::value,
						 int>::type = 0>
		Json(const V &v) : Json(_array(v.begin(), v.end())) {}

	public:
		// 析构函数
		~Json();

	public:
		// 拷贝
		Json(const Json &);                    // 深拷贝
		Json &operator=(const Json &) noexcept;// 拷贝，交换

	public:
		// 移动
		Json(Json &&) noexcept;
		Json &operator=(Json &&) noexcept;

	public:
		// 数据类型判断接口
		JsonValueType getType() const noexcept;
		bool isNull() const noexcept;
		bool isBoolean() const noexcept;
		bool isNumber() const noexcept;
		bool isString() const noexcept;
		bool isArray() const noexcept;
		bool isObject() const noexcept;

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

	public:
		// 数组和对象数据接口
		size_t size() const;
		// 数组
		Json &operator[](size_t);
		const Json &operator[](size_t) const;
		// 对象
		Json &operator[](const std::string &);
		const Json &operator[](const std::string &) const;

	private:
		std::string serializeString() const noexcept;
		std::string serializeArray() const noexcept;
		std::string serializeObject() const noexcept;

	private:
		std::unique_ptr<JsonValue> _value;// PIMPL
	};

	inline std::ostream &operator<<(std::ostream &os, const Json &json) {
		return os << json.serialize();
	}
	bool operator==(const Json &, const Json &);
	inline bool operator!=(const Json &lhs, const Json &rhs) {
		return !(lhs == rhs);
	}
}// namespace DianaJSON

#endif