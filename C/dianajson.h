#ifndef DIANAJSON_H
#define DIANAJSON_H

#include <stddef.h> /* size_t */

/* Json 7种数据类型 */
/* 使用项目简写作为标识符的前缀（C无CPP命名空间功能） */
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

/* JSON数据结构 */
/* 树形结构 */
typedef struct diana_value diana_value;
typedef struct diana_member diana_member;

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

struct diana_member
{
    char *k;     /* member key string */
    size_t klen; /* key string length */

    diana_value v; /* member value */
};

/* JSON解析返回值 */
enum
{
    DIANA_PARSE_OK = 0,
    DIANA_PARSE_EXPECT_VALUE,      // JSON只含有空白
    DIANA_PARSE_INVALID_VALUE,     // 属于非法字面值
    DIANA_PARSE_ROOT_NOT_SINGULAR, // 若一个值之后，在空白之后还有其他字符
    DIANA_PARSE_NUMBER_TOO_BIG,    // 数字过大
    DIANA_PARSE_MISS_QUOTATION_MARK,
    DIANA_PARSE_INVALID_STRING_ESCAPE,
    DIANA_PARSE_INVALID_STRING_CHAR,
    DIANA_PARSE_INVALID_UNICODE_SURROGATE, //缺乏高代理项或低代理项不合法
    DIANA_PARSE_INVALID_UNICODE_HEX,       //\u后不是4位十六进制数字
    DIANA_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    DIANA_PARSE_MISS_KEY,
    DIANA_PARSE_MISS_COLON,
    DIANA_PARSE_MISS_COMMA_OR_CURLY_BRACKET,
    DIANA_STRINGIFY_OK
};

#define diana_init(v)           \
    do                          \
    {                           \
        (v)->type = DIANA_NULL; \
    } while (0) // 初始化类型

/* JSON解析 */
int diana_parse(diana_value *v, const char *json); // 解析JSON，根节点指针v是由使用方负责分配

void diana_free(diana_value *v);

/* 获取访问结果函数，获取其类型 */
diana_type diana_get_type(const diana_value *v);

/* 相等比较 */
int diana_is_equal(const diana_value *lhs, const diana_value *rhs);

#define diana_set_null(v) diana_free(v)

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
void diana_clear_array(diana_value *v);                                     // 清楚所有元素（不改变容量）

/* 对象类型 */
void diana_set_object(diana_value *v, size_t capacity); // 设置对象的函数，提供初始容量
void diana_reserve_object();                            // 扩大容量
void diana_shrink_array(diana_value *v);                // 对象瘦身
void diana_clear_object(diana_value *v);
size_t diana_get_object_size(const diana_value *v);
size_t diana_get_object_capacity(const diana_value *v);
const char *diana_get_object_key(const diana_value *v, size_t index);
size_t diana_get_object_key_length(const diana_value *v, size_t index);
diana_value *diana_get_object_value(const diana_value *v, size_t index);
size_t diana_find_object_index(const diana_value *v, const char *key, size_t klen);
diana_value *diana_find_object_value(diana_value *v, const char *key, size_t klen);
diana_value *diana_set_object_value(diana_value *v, const char *key, size_t klen, const diana_value *value);
void diana_remove_object_value(diana_value *v, size_t index);

/* 深度复制 */
void diana_copy(diana_value *dst, const diana_value *src);

/* move */
void diana_move(diana_value *dst, diana_value *src);

/* 交换值 */
void diana_swap(diana_value *lhs, diana_value *rhs);

/* 生成器 */
char *diana_stringify(const diana_value *v, size_t *length);

#endif /* DIANAJSON_H */