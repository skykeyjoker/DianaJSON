#include "json.h"
//#include "parse.h"
#include "jsonvalue.h"

#include <cstdio>

namespace DianaJSON
{

    // 数据接口
    JsonValueType Json::getType() const noexcept
    {
        return _value->getType();
    }
    bool Json::isNull() const
    {
        return _value->getType() == JsonValueType::Null;
    }
    bool Json::isBoolean() const
    {
        return _value->getType() == JsonValueType::Bool;
    }
    bool Json::isNumber() const
    {
        return _value->getType() == JsonValueType::Number;
    }
    bool Json::isString() const
    {
        return _value->getType() == JsonValueType::String;
    }
    bool Json::isArray() const
    {
        return _value->getType() == JsonValueType::Array;
    }
    bool Json::isObject() const
    {
        return _value->getType() == JsonValueType::Object;
    }

    bool Json::toBool() const
    {
        return _value->toBool();
    }
    double Json::toDouble() const
    {
        return _value->toDouble();
    }
    const std::string &Json::toString() const
    {
        return _value->toString();
    }
    const Json::_array &Json::toArray() const
    {
        return _value->toArray();
    }
    const Json::_object &Json::toObject() const
    {
        return _value->toObject();
    }

}