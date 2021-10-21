# DianaJSON

关注嘉然，顿顿解馋。

C++版本的简易JSON解析器实现。

## 设计
统一利用命名空间`namespace DianaJSON`进行作用域隔离。

对外提供
* Json类作为JSON解析和生成接口。
* JsonError类作为错误信息类。
内部
* JsonValue类作为Json类内部私有数据方法类，是JSON的具体数据。
* Parser类作为JSON解析和实现具体实现的方法类。

### Json类
#### 定义数据类型
声明JSON数据类型：
* Null
* Bool
* Number
* String
* Array（使用std::vector储存）
* Object（使用std::unorderd_map储存）

#### 构造函数，析构函数等

#### 接口
数据类型判断接口：
```cpp
JsonValueType getType() const;
bool isNull() const;
bool isBoolean() const;
bool isNumber() const;
bool isString() const;
bool isArray() const;
bool isObject() const;
```
数据类型转换接口：
```cpp
bool toBool() const;
double toDouble() const; // Number类型实际表现为double类型浮点数
std::string &toString() const;
_array &toArray() const;
_object &toObject() const;
```
Number类型实际表现为double类型浮点数。

以上两种数据类型接口实际转入内部类_value内进行操作。

#### PIMPL模式
使用PIMPL设计模式，JsonValue为内部类。

PIMPL可形成编译防火墙，加速构建并令程序解耦合。

### JsonValue类
#### PIMPL模式
PIMPL设计模式，作为Json内部类。

#### 接口
具体实现数据类型的判断和转换接口。
数据类型判断接口：
```cpp
JsonValueType getType() const;
bool isNull() const;
bool isBoolean() const;
bool isNumber() const;
bool isString() const;
bool isArray() const;
bool isObject() const;
```
数据类型转换接口：
```cpp
bool toBool() const;
double toDouble() const; // Number类型实际表现为double类型浮点数
std::string &toString() const;
_array &toArray() const;
_object &toObject() const;
```




