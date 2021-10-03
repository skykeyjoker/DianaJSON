#include "dianajson.h"
#include <assert.h> /* assert() */
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>  /* errno, ERANGE */
#include <math.h>   /* HUGE_VAL */
#include <string.h> /* memcpy() */

/* 使用者可在编译选项中自行设置DIANA_PARSE_STACK_INIT_SIZE宏 */
#ifndef DIANA_PARSE_STACK_INIT_SIZE
#define DIANA_PARSE_STACK_INIT_SIZE 256
#endif

#define EXPECT(c, ch)             \
    do                            \
    {                             \
        assert(*c->json == (ch)); \
        c->json++;                \
    } while (0)

#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

/* 减少解析函数之间传递多个参数 */
typedef struct
{
    const char *json;
    char *stack;
    size_t size, top; // size当前堆栈容量，top栈顶位置
} diana_context;

static void *diana_context_push(diana_context *c, size_t size)
{
    void *ret;
    assert(size > 0);
    if (c->top + size >= c->size)
    {
        if (c->size == 0)
            c->size = DIANA_PARSE_STACK_INIT_SIZE;
        while (c->top + size >= c->size)
            c->size += c->size >> 1; // 1.5倍扩展
        c->stack = (char *)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top; // 返回数据起始的指针
    c->top += size;
    return ret;
}

static void *diana_context_pop(diana_context *c, size_t size)
{
    assert(c->top >= size);
    return c->stack + (c->top -= size);
}

#define PUTC(c, ch)                                          \
    do                                                       \
    {                                                        \
        *(char *)diana_context_push(c, sizeof(char)) = (ch); \
    } while (0)

/* 处理白空格ws */
/* ws = *(%x20 / %x09 / %x0A / %x0D) */
static void diana_parse_whitespace(diana_context *c)
{
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

/* 统一解析null/false/true */
/* null = "null" */
/* true = "true" */
/* false = "false" */
static int diana_parse_literal(diana_context *c, diana_value *v, const char *literal, diana_type type)
{
    size_t i;
    EXPECT(c, literal[0]);
    for (i = 0; literal[i + 1]; i++)
    {
        if (c->json[i] != literal[i + 1])
            return DIANA_PARSE_INVALID_VALUE;
    }
    c->json += i;
    v->type = type;
    return DIANA_PARSE_OK;
}

/* 解析数值 */
static int diana_parse_number(diana_context *c, diana_value *v)
{
    /* 检验数字 */
    /* 负号 整数 小数 指数 */
    /**
     * number = [ "-" ] int [ frac ] [ exp ]
     * int = "0" / digit1-9 *digit
     * frac = "." 1*digit
     * exp = ("e" / "E") ["-" / "+"] 1*digit 
     */
    const char *p = c->json;

    // 检测负号
    if (*p == '-')
        p++;

    /* 检测整数 */
    /**
     * 两种情况，单个0或者1-9再加任意数量的digit
     */
    if (*p == '0')
        p++;
    else
    {
        if (!ISDIGIT1TO9(*p))
            return DIANA_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++)
        {
        }
    }

    /* 检测小数 */
    if (*p == '.')
    {
        p++;
        if (!ISDIGIT(*p))
            return DIANA_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++)
        {
        }
    }

    /* 检测指数 */
    if (*p == 'e' || *p == 'E')
    {
        p++;
        if (*p == '+' || *p == '-')
            p++;
        if (!ISDIGIT(*p))
            return DIANA_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++)
        {
        }
    }

    errno = 0;
    v->u.n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
        return DIANA_PARSE_NUMBER_TOO_BIG;
    v->type = DIANA_NUMBER;
    c->json = p;
    return DIANA_PARSE_OK;
}

/* 解析字符串 */
static int diana_parse_string(diana_context *c, diana_value *v)
{
    size_t head = c->top, len;
    const char *p;
    EXPECT(c, '\"');
    p = c->json;
    for (;;)
    {
        char ch = *p++;
        switch (ch)
        {
        case '\"':
            len = c->top - head;
            diana_set_string(v, (const char *)diana_context_pop(c, len), len);
            c->json = p;
            return DIANA_PARSE_OK;
        case '\0':
            c->top = head;
            return DIANA_PARSE_MISS_QUOTATION_MARK;
        default:
            PUTC(c, ch);
        }
    }
}

/* value = null / false / true / number / string */
static int diana_parse_value(diana_context *c, diana_value *v)
{
    switch (*c->json)
    {
    case 't':
        return diana_parse_literal(c, v, "true", DIANA_TRUE);
    case 'f':
        return diana_parse_literal(c, v, "false", DIANA_FALSE);
    case 'n':
        return diana_parse_literal(c, v, "null", DIANA_NULL);
    default:
        return diana_parse_number(c, v);
    case '"':
        return diana_parse_string(c, v);
    case '\0':
        return DIANA_PARSE_EXPECT_VALUE;
    }
}

/* 格式：JSON-text = ws value ws */
/* 递归下降解析器 */
int diana_parse(diana_value *v, const char *json)
{
    diana_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    /* 初始化堆栈 */
    c.stack = NULL;
    c.size = c.top = 0;
    diana_init(v);
    diana_parse_whitespace(&c);
    if ((ret = diana_parse_value(&c, v)) == DIANA_PARSE_OK)
    {
        diana_parse_whitespace(&c); // 检测第三部分
        if (*c.json != '\0')
        {
            v->type = DIANA_NULL;
            ret = DIANA_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    assert(c.top == 0);
    free(c.stack);
    return ret;
}

void diana_free(diana_value *v)
{
    assert(v != NULL);
    if (v->type == DIANA_STRING)
        free(v->u.s.s);
    v->type = DIANA_NULL; // 类型置为null，避免重复释放
}

diana_type diana_get_type(const diana_value *v)
{
    assert(v != NULL);
    return v->type;
}

int diana_get_boolean(const diana_value *v)
{
    /* TODO */
    assert(v != NULL && (v->type == DIANA_TRUE || v->type == DIANA_FALSE));
    return v->type == DIANA_TRUE ? 1 : 0;
}

void diana_set_boolean(diana_value *v, int b)
{
    /* TODO */
    assert(v != NULL && (b == 0 || b == 1));
    v->type = (b == 1 ? DIANA_TRUE : DIANA_FALSE);
}

double diana_get_number(const diana_value *v)
{
    assert(v != NULL && v->type == DIANA_NUMBER);
    return v->u.n;
}

void diana_set_number(diana_value *v, double n)
{
    /* TODO */
    assert(v != NULL);
    v->u.n = n;
    v->type = DIANA_NUMBER;
}
const char *diana_get_string(const diana_value *v)
{
    assert(v != NULL && v->type == DIANA_STRING);
    return v->u.s.s;
}
size_t diana_get_string_length(const diana_value *v)
{
    assert(v != NULL && v->type == DIANA_STRING);
    return v->u.s.len;
}

void diana_set_string(diana_value *v, const char *s, size_t len)
{
    assert(v != NULL && (s != NULL || len == 0)); // 非空指针或者零长度的字符串都是合法的
    diana_free(v);                                // 首先清空v可能分配到的内存
    v->u.s.s = (char *)malloc(len + 1);           // 分配字符串内存
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = DIANA_STRING;
}
