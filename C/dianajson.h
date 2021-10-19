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
            size_t size;
        } o;
        struct /* array */
        {
            diana_value *e;
            size_t size;
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
    DIANA_PARSE_MISS_COMMA_OR_CURLY_BRACKET
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
size_t diana_get_array_size(const diana_value *v);
diana_value *diana_get_array_element(const diana_value *v, size_t index);

/* 对象类型 */
size_t diana_get_object_size(const diana_value *v);
const char *diana_get_object_key(const diana_value *v, size_t index);
size_t diana_get_object_key_length(const diana_value *v, size_t index);
diana_value *diana_get_object_value(const diana_value *v, size_t index);

#endif /* DIANAJSON_H */