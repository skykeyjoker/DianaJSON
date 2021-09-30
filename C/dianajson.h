#ifndef DIANAJSON_H
#define DIANAJSON_H

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
typedef struct
{
    double n;
    diana_type type;
} diana_value; // 树形结构的每个节点使用diana_value表示，也称它为一个值（JSON Value）。

/* JSON解析返回值 */
enum
{
    DIANA_PARSE_OK = 0,
    DIANA_PARSE_EXPECT_VALUE,      // JSON只含有空白
    DIANA_PARSE_INVALID_VALUE,     // 属于非法字面值
    DIANA_PARSE_ROOT_NOT_SINGULAR, // 若一个值之后，在空白之后还有其他字符
    DIANA_PARSE_NUMBER_TOO_BIG     // 数字过大
};

/* JSON解析 */
int diana_parse(diana_value *v, const char *json); // 解析JSON，根节点指针v是由使用方负责分配

/* 获取访问结果函数，获取其类型 */
diana_type diana_get_type(const diana_value *v);

/* 获取数值型数据API */
double diana_get_number(const diana_value *v);

#endif /* DIANAJSON */