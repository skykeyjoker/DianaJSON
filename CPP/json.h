#ifndef JSON_H
#define JSON_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

namespace DianaJSON
{
    // 声明JSON数据类型
    enum class JsonValueType
    {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    // 为JsonValue内部类前向声明（std::unique_ptr）
    class JsonValue;

    class Json
    {
    public:
        // 类型重名
        using _array = std::vector<Json>;
        using _object = std::unordered_map<std::string, Json>;

    public:
        // 构造函数等预留

    public:
        // 数据类型判断接口
        JsonValueType getType() const noexcept;
        bool isNull() const;
        bool isBoolean() const;
        bool isNumber() const;
        bool isString() const;
        bool isArray() const;
        bool isObject() const;

    public:
        // 数据类型转换接口
        bool toBool() const;
        double toDouble() const; // Number类型实际表现为double类型浮点数
        const std::string &toString() const;
        const _array &toArray() const;
        const _object &toObject() const;

    private:
        std::unique_ptr<JsonValue> _value; // PIMPL
    };
}

#endif