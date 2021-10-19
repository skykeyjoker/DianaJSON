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
#define STRING_ERROR(ret) \
    do                    \
    {                     \
        c->top = head;    \
        return ret;       \
    } while (0)

/* 读取4位16进制数字 */
static const char *diana_parse_hex4(const char *p, unsigned *u)
{
    int i;
    *u = 0;
    for (i = 0; i < 4; ++i)
    {
        char ch = *p++;
        *u <<= 4;
        if (ch >= '0' && ch <= '9')
            *u |= ch - '0';
        else if (ch >= 'A' && ch <= 'F')
            *u |= ch - ('A' - 10);
        else if (ch >= 'a' && ch <= 'f')
            *u |= ch - ('a' - 10);
        else
            return NULL;
    }
    return p;
}

/* 转码为utf8 */
static void diana_encode_utf8(diana_context *c, unsigned u)
{
    if (u <= 0x7F)
        PUTC(c, u & 0xFF);
    else if (u <= 0x7FF)
    {
        PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
        PUTC(c, 0x80 | (u & 0x3F));
    }
    else if (u <= 0xFFFF)
    {
        PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
        PUTC(c, 0x80 | ((u >> 6) & 0x3F));
        PUTC(c, 0x80 | (u & 0x3F));
    }
    else
    {
        assert(u <= 0x10FFFF);
        PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
        PUTC(c, 0x80 | ((u >> 12) & 0x3F));
        PUTC(c, 0x80 | ((u >> 6) & 0x3F));
        PUTC(c, 0x80 | (u & 0x3F));
    }
}

/* 解析JSON字符串，把结果写入str和len */
/* str只想c->stack中的元素 */
static int diana_parse_string_raw(diana_context *c, char **str, size_t *len)
{
    /* TODO */
    unsigned u, u2;
    size_t head = c->top;
    const char *p;
    EXPECT(c, '\"');
    p = c->json;
    for (;;)
    {
        char ch = *p++;
        switch (ch)
        {
        case '\"':
            *len = c->top - head;
            // diana_set_string(v, (const char *)diana_context_pop(c, *len), *len);
            *str = diana_context_pop(c, *len);
            c->json = p;
            return DIANA_PARSE_OK;
        case '\\':
            switch (*p++)
            {
            case '\"':
                PUTC(c, '\"');
                break;
            case '\\':
                PUTC(c, '\\');
                break;
            case '/':
                PUTC(c, '/');
                break;
            case 'b':
                PUTC(c, '\b');
                break;
            case 'f':
                PUTC(c, '\f');
                break;
            case 'n':
                PUTC(c, '\n');
                break;
            case 'r':
                PUTC(c, '\r');
                break;
            case 't':
                PUTC(c, '\t');
                break;
            case 'u':
                if (!(p = diana_parse_hex4(p, &u)))
                    STRING_ERROR(DIANA_PARSE_INVALID_UNICODE_HEX);
                /* surrogate handling */
                if (u >= 0xD800 && u <= 0xDBFF)
                { /* surrogate pair */
                    if (*p++ != '\\')
                        STRING_ERROR(DIANA_PARSE_INVALID_UNICODE_SURROGATE);
                    if (*p++ != 'u')
                        STRING_ERROR(DIANA_PARSE_INVALID_UNICODE_SURROGATE);
                    if (!(p = diana_parse_hex4(p, &u2)))
                        STRING_ERROR(DIANA_PARSE_INVALID_UNICODE_HEX);
                    if (u2 < 0xDC00 || u2 > 0xDFFF)
                        STRING_ERROR(DIANA_PARSE_INVALID_UNICODE_SURROGATE);
                    u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                }
                diana_encode_utf8(c, u);
                break;
            default:
                STRING_ERROR(DIANA_PARSE_INVALID_STRING_ESCAPE);
            }
            break;
        case '\0':
            STRING_ERROR(DIANA_PARSE_MISS_QUOTATION_MARK);
        default:
            if ((unsigned char)ch < 0x20)
                STRING_ERROR(DIANA_PARSE_INVALID_STRING_CHAR);
            PUTC(c, ch);
        }
    }
}

static int diana_parse_string(diana_context *c, diana_value *v)
{
    int ret;
    char *s;
    size_t len;
    if ((ret = diana_parse_string_raw(c, &s, &len)) == DIANA_PARSE_OK)
        diana_set_string(v, s, len);
    return ret;
}

static int diana_parse_value(diana_context *c, diana_value *v); /* 前向声明 */

/* 解析数组 */
static int diana_parse_array(diana_context *c, diana_value *v)
{
    size_t i, size = 0;
    int ret;
    EXPECT(c, '[');
    diana_parse_whitespace(c);
    if (*c->json == ']')
    {
        c->json++;
        v->type = DIANA_ARRAY;
        v->u.a.size = 0;
        v->u.a.e = NULL;
        return DIANA_PARSE_OK;
    }
    for (;;)
    {
        diana_value e;
        diana_init(&e);
        if ((ret = diana_parse_value(c, &e)) != DIANA_PARSE_OK)
            break;
        memcpy(diana_context_push(c, sizeof(diana_value)), &e, sizeof(diana_value));
        size++;
        diana_parse_whitespace(c);
        if (*c->json == ',')
        {
            c->json++;
            diana_parse_whitespace(c);
        }
        else if (*c->json == ']')
        {
            c->json++;
            v->type = DIANA_ARRAY;
            v->u.a.size = size;
            size *= sizeof(diana_value);
            memcpy(v->u.a.e = (diana_value *)malloc(size), diana_context_pop(c, size), size);
            return DIANA_PARSE_OK;
        }
        else
        {
            ret = DIANA_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }

    /* Pop and free values on the stack */
    for (i = 0; i < size; i++)
        diana_free((diana_value *)diana_context_pop(c, sizeof(diana_value)));
    return ret;
}

/* 解析对象 */
/* object = %x7B ws [ member *( ws %x2C ws member ) ] ws %x7D */
static int diana_parse_object(diana_context *c, diana_value *v)
{
    size_t i, size;
    diana_member m;
    int ret;
    EXPECT(c, '{');
    diana_parse_whitespace(c);
    if (*c->json == '}') // 空对象
    {
        c->json++;
        v->type = DIANA_OBJECT;
        v->u.o.m = 0;
        v->u.o.size = 0;
        return DIANA_PARSE_OK;
    }
    m.k = NULL;
    size = 0;
    for (;;)
    {
        char *str;
        diana_init(&m.v);
        /* parse key to m.k, m.klen */
        if (*c->json != '"')
        {
            ret = DIANA_PARSE_MISS_KEY;
            break;
        }
        if ((ret = diana_parse_string_raw(c, &str, &m.klen)) != DIANA_PARSE_OK)
            break;
        memcpy(m.k = (char *)malloc(m.klen + 1), str, m.klen);
        m.k[m.klen] = '\0';
        /* parse ws colon ws */
        diana_parse_whitespace(c);
        if (*c->json != ':')
        {
            ret = DIANA_PARSE_MISS_COLON;
            break;
        }
        c->json++;
        diana_parse_whitespace(c);
        /* parse value */
        if ((ret = diana_parse_value(c, &m.v)) != DIANA_PARSE_OK)
            break;
        memcpy(diana_context_push(c, sizeof(diana_member)), &m, sizeof(diana_member));
        size++;
        m.k = NULL; /* ownership is transferred to member on stack */
        /* parse ws [comma | right-curly-brace] ws */
        diana_parse_whitespace(c);
        if (*c->json == ',')
        {
            c->json++;
            diana_parse_whitespace(c);
        }
        else if (*c->json == '}')
        {
            size_t s = sizeof(diana_member) * size;
            c->json++;
            v->type = DIANA_OBJECT;
            v->u.o.size = size;
            memcpy(v->u.o.m = (diana_member *)malloc(s), diana_context_pop(c, s), s);
            return DIANA_PARSE_OK;
        }
        else
        {
            ret = DIANA_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
    }
    /* Pop and free members on the stack */
    free(m.k);
    for (i = 0; i < size; i++)
    {
        diana_member *m = (diana_member *)diana_context_pop(c, sizeof(diana_member));
        free(m->k);
        diana_free(&m->v);
    }
    v->type = DIANA_NULL;
    return ret;
}

/* value = null / false / true / number / string / array */
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
    case '[':
        return diana_parse_array(c, v);
    case '{':
        return diana_parse_object(c, v);
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
    size_t i;
    assert(v != NULL);
    switch (v->type)
    {
    case DIANA_STRING:
        free(v->u.s.s);
        break;
    case DIANA_ARRAY:
        for (i = 0; i < v->u.a.size; ++i)
            diana_free(&v->u.a.e[i]);
        free(v->u.a.e);
        break;
    case DIANA_OBJECT:
        for (i = 0; i < v->u.o.size; i++)
        {
            free(v->u.o.m[i].k);
            diana_free(&v->u.o.m[i].v);
        }
        free(v->u.o.m);
        break;
    default:
        break;
    }
    v->type = DIANA_NULL; // 类型置为null，避免重复释放
}

diana_type diana_get_type(const diana_value *v)
{
    assert(v != NULL);
    return v->type;
}

int diana_get_boolean(const diana_value *v)
{
    assert(v != NULL && (v->type == DIANA_TRUE || v->type == DIANA_FALSE));
    return v->type == DIANA_TRUE ? 1 : 0;
}

void diana_set_boolean(diana_value *v, int b)
{
    diana_free(v);
    v->type = (b == 1 ? DIANA_TRUE : DIANA_FALSE);
}

double diana_get_number(const diana_value *v)
{
    assert(v != NULL && v->type == DIANA_NUMBER);
    return v->u.n;
}

void diana_set_number(diana_value *v, double n)
{
    diana_free(v);
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
size_t diana_get_array_size(const diana_value *v)
{
    assert(v != NULL && v->type == DIANA_ARRAY);
    return v->u.a.size;
}

diana_value *diana_get_array_element(const diana_value *v, size_t index)
{
    assert(v != NULL && v->type == DIANA_ARRAY);
    assert(index < v->u.a.size);
    return &v->u.a.e[index];
}

size_t diana_get_object_size(const diana_value *v)
{
    assert(v != NULL && v->type == DIANA_OBJECT);
    return v->u.o.size;
}

const char *diana_get_object_key(const diana_value *v, size_t index)
{
    assert(v != NULL && v->type == DIANA_OBJECT);
    assert(index < v->u.o.size);
    return v->u.o.m[index].k;
}

size_t diana_get_object_key_length(const diana_value *v, size_t index)
{
    assert(v != NULL && v->type == DIANA_OBJECT);
    assert(index < v->u.o.size);
    return v->u.o.m[index].klen;
}

diana_value *diana_get_object_value(const diana_value *v, size_t index)
{
    assert(v != NULL && v->type == DIANA_OBJECT);
    assert(index < v->u.o.size);
    return &v->u.o.m[index].v;
}