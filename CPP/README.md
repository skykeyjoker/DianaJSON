# DianaJSON

关注嘉然，顿顿解馋。

C++版本的简易JSON解析器实现。

## 设计

统一利用命名空间`namespace DianaJSON`进行作用域隔离。

对外提供

* Json类作为JSON解析和生成接口。
* JsonError类作为错误信息类。 内部
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

#### 构造函数&析构函数等

对几种数据分别特化构造函数，对String、Array和Object类型的数据还要特化移动构造函数。析构函数使用默认版本即可。

```cpp
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
```

同时细化拷贝构造函数、拷贝赋值，移动构造函数、移动赋值。

```cpp
public:
	// 拷贝
	Json(const Json &);                    // 深拷贝
	Json &operator=(const Json &) noexcept;// 拷贝，交换

public:
	// 移动
	Json(Json &&) noexcept;
	Json *operator=(Json &&) noexcept;
```

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
const std::string &toString() const;
const _array &toArray() const;
const _object &toObject() const;
```

Number类型实际表现为double类型浮点数。

以上两种数据类型接口实际转入内部类_value内进行操作。

JSON解析与生成接口：

```cpp
// 解析与生成器接口
static Json parse(const std::string &context, std::string &errorText) noexcept;// 解析
std::string serialize() const noexcept;                                        // 生成器
```

#### PIMPL模式

使用PIMPL设计模式，JsonValue为内部类。

PIMPL可形成编译防火墙，加速构建并令程序解耦合。

### JsonValue类

JsonValue作为Json的内部类，具体实现对JSON值的存储和转换。

#### PIMPL模式

PIMPL设计模式，作为Json内部类。

#### 构造函数&析构函数等

对几种数据类型分别特化构造函数，对String、Array和Object类型的数据还要特化移动构造函数。析构函数使用默认版本即可。

```cpp
public:
    // 构造函数
    JsonValue(std::nullptr_t) : _val(nullptr) {}
    JsonValue(bool val) : _val(val) {}
    JsonValue(double val) : _val(val) {}
    JsonValue(const std::string &val) : _val(val) {}
    JsonValue(const Json::_array &val) : _val(val) {}
    JsonValue(const Json::_object &val) : _val(val) {}

public:
    // 移动构造函数
    JsonValue(std::string &&val) : _val(std::move(val)) {}
    JsonValue(Json::_array &&val) : _val(std::move(val)) {}
    JsonValue(Json::_object &&val) : _val(std::move(val)) {}

public:
    // 析构函数
    ~JsonValue() = default;
```

#### 接口

具体实现数据类型的判断和转换接口。 数据类型判断接口：

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
const std::string &toString() const;
const _array &toArray() const;
const _object &toObject() const;
```

数组对象接口：

```cpp
// Array与Object类的访问接口
size_t size() const; // 返回数组或对象成员数

// 数组，支持随机访问
const Json &operator[](size_t index) const;
Json &operator[](size_t index);

// 对象，O(1)遍历寻找（不支持插入）
const Json &operator[](const std::string &key) const;
Json &operator[](const std::string &key);
```

其中，const与非const方法为减少重复代码，仅具体实现const方法后，使用`const_cast`进行转换。




