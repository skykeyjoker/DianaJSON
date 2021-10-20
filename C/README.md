# DianaJSON

关注嘉然，顿顿解馋。

C版本的简易JSON解析器实现。

本实现特点如下：

- 符合标准的 JSON 解析器和生成器
- 手写的递归下降解析器（recursive descent parser）
- 使用标准 C 语言（C89）
- 跨平台／编译器（如 Windows／Linux／OS X，vc／gcc／clang）
- 仅支持 UTF-8 JSON 文本
- 仅支持以 `double` 存储 JSON number 类型



## 数据类型

共实现了七种数据的解析。

| 数据类型     | 说明               |
| ------------ | ------------------ |
| DIANA_NULL   | NULL               |
| DIANA_FALSE  | FALSE              |
| DIANA_TRUE   | TRUE               |
| DIANA_NUMBER | 数值（`double`）   |
| DIANA_STRING | 字符串（`char *`） |
| DIANA_ARRAY  | 数组               |
| DIANA_OBJECT | 对象               |

使用`typedef`定义了这七种数据类型：

```cpp
typedef enum
{
    DIANA_NULL,
    DIANA_FALSE,
    DIANA_TRUE,
    DIANA_NUMBER,
    DIANA_STRING,
    DIANA_ARRAY,
    DIANA_OBJECT
} diana_type; // JSON数据类型
```

七种数据类型共同储存在`diana_value`结构体中，为了节省储存空间，使用了union来储存不同类型。

```cpp
struct diana_value
{
    union
    {
        struct /* member */
        {
            diana_member *m;
            size_t size;     // member count
            size_t capacity; // capacity
        } o;
        struct /* array */
        {
            diana_value *e;
            size_t size;     // element count
            size_t capacity; // capacity
        } a;
        struct /* string */
        {
            char *s;
            size_t len;
        } s;
        double n; /* number */
    } u;
    diana_type type;
}; // 树形结构的每个节点使用diana_value表示，也称它为一个值（JSON Value）。
```





## 接口

```cpp
/* JSON解析 */
int diana_parse(diana_value *v, const char *json); // 解析JSON，根节点指针v是由使用方负责分配

/* 释放数据 */
void diana_free(diana_value *v);

/* 获取访问结果函数，获取其类型 */
diana_type diana_get_type(const diana_value *v);

/* 相等比较 */
int diana_is_equal(const diana_value *lhs, const diana_value *rhs);

/* Boolean类型数据 */
int diana_get_boolean(const diana_value *v);
void diana_set_boolean(diana_value *v, int b);

/* 数值型数据 */
double diana_get_number(const diana_value *v);
void diana_set_number(diana_value *v, double n);

/* 字符型数据 */
const char *diana_get_string(const diana_value *v);
size_t diana_get_string_length(const diana_value *v);
void diana_set_string(diana_value *v, const char *s, size_t len);

/* 数组类型数据 */
void diana_set_array(diana_value *v, size_t capacity);     // 设置数组的函数，提供初始容量
void diana_reserve_array(diana_value *v, size_t capacity); // 扩大容量
void diana_shrink_array(diana_value *v);                   // 数组瘦身
size_t diana_get_array_size(const diana_value *v);
size_t diana_get_array_capacity(const diana_value *v);
diana_value *diana_get_array_element(const diana_value *v, size_t index);
diana_value *diana_pushback_array_element(diana_value *v);                  // 数组末端加入元素，返回新的元素指针
void diana_popback_array_element(diana_value *v);                           // 删去数组末端元素
diana_value *diana_insert_array_element(diana_value *v, size_t index);      // 在index位置插入一个元素，返回新的元素指针
void diana_erase_array_element(diana_value *v, size_t index, size_t count); // 删去在index位置开始共count个元素（不改变容量）
void diana_clear_array(diana_value *v);                                     // 清除所有元素（不改变容量）

/* 对象类型 */
void diana_set_object(diana_value *v, size_t capacity);     // 设置对象的函数，提供初始容量
void diana_reserve_object(diana_value *v, size_t capacity); // 扩大容量
void diana_shrink_object(diana_value *v);                   // 对象瘦身
void diana_clear_object(diana_value *v);
size_t diana_get_object_size(const diana_value *v);
size_t diana_get_object_capacity(const diana_value *v);
const char *diana_get_object_key(const diana_value *v, size_t index);
size_t diana_get_object_key_length(const diana_value *v, size_t index);
diana_value *diana_get_object_value(const diana_value *v, size_t index);
size_t diana_find_object_index(const diana_value *v, const char *key, size_t klen);
diana_value *diana_find_object_value(diana_value *v, const char *key, size_t klen);
diana_value *diana_set_object_value(diana_value *v, const char *key, size_t klen); // 设置键值对，先搜寻是否存在现有的键，若存在则直接返回该值的指针，不存在时才新增。
void diana_remove_object_value(diana_value *v, size_t index);

/* 深度复制 */
void diana_copy(diana_value *dst, const diana_value *src);

/* move */
void diana_move(diana_value *dst, diana_value *src);

/* 交换值 */
void diana_swap(diana_value *lhs, diana_value *rhs);

/* 生成器 */
char *diana_stringify(const diana_value *v, size_t *length);
```

