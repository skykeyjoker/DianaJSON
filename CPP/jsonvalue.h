#ifndef JSONVALUE_H
#define JSONVALUE_H

#include "json.h"
//#include "jsonerror.h"

#include <variant>

namespace DianaJSON
{
    // 实现内部类JsonValue
    class JsonValue
    {
    public:
    public:
        // 数据类型判断接口
        JsonValueType getType() const;
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
        std::string &toString() const;
        Json::_array &toArray() const;
        Json::_object &toObject() const;
    };
}

#endif