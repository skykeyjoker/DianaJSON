#include "dianajson.h"
#include <assert.h> /* assert() */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>  /* sprintf */
#include <errno.h>  /* errno, ERANGE */
#include <math.h>   /* HUGE_VAL */
#include <string.h> /* memcpy() */

/* 使用者可在编译选项中自行设置DIANA_PARSE_STACK_INIT_SIZE宏 */
#ifndef DIANA_PARSE_STACK_INIT_SIZE
#define DIANA_PARSE_STACK_INIT_SIZE 256
#endif

/* 使用者可在编译选项中自行设置DIANA_PARSE_STRINGIFY_INIT_SIZE宏 */
#ifndef DIANA_PARSE_STRINGIFY_INIT_SIZE
#define DIANA_PARSE_STRINGIFY_INIT_SIZE 256
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
        // v->type = DIANA_ARRAY;
        // v->u.a.size = 0;
        // v->u.a.e = NULL;
        diana_set_array(v, 0);
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
            // v->type = DIANA_ARRAY;
            // v->u.a.size = size;
            // size *= sizeof(diana_value);
            diana_set_array(v, size);
            // memcpy(v->u.a.e = (diana_value *)malloc(size), diana_context_pop(c, size), size);
            memcpy(v->u.a.e, diana_context_pop(c, size * sizeof(diana_value)), size * sizeof(diana_value));
            v->u.a.size = size;
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
        // v->type = DIANA_OBJECT;
        // v->u.o.m = 0;
        // v->u.o.size = 0;
        diana_set_object(v, 0);
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
            // size_t s = sizeof(diana_member) * size;
            c->json++;
            // v->type = DIANA_OBJECT;
            // v->u.o.size = size;
            diana_set_object(v, size);
            // memcpy(v->u.o.m = (diana_member *)malloc(s), diana_context_pop(c, s), s);
            memcpy(v->u.o.m, diana_context_pop(c, sizeof(diana_member) * size), sizeof(diana_member) * size);
            v->u.o.size = size;
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
            v->u.o.m[i].klen = 0;
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

void diana_set_array(diana_value *v, size_t capacity)
{
    assert(v != NULL);
    diana_free(v);
    v->type = DIANA_ARRAY;
    v->u.a.size = 0;
    v->u.a.capacity = capacity;
    v->u.a.e = capacity > 0 ? (diana_value *)malloc(capacity * sizeof(diana_value)) : NULL;
}

void diana_reserve_array(diana_value *v, size_t capacity)
{
    if (v == NULL)
        printf("v is null!\n");
    if (v->type != DIANA_ARRAY)
        printf("v->type != DIANA_ARRAY!\n");
    assert(v != NULL && v->type == DIANA_ARRAY);
    if (v->u.a.capacity < capacity)
    {
        v->u.a.capacity = capacity;
        v->u.a.e = (diana_value *)realloc(v->u.a.e, capacity * sizeof(diana_value));
    }
}

void diana_shrink_array(diana_value *v)
{
    assert(v != NULL && v->type == DIANA_ARRAY);
    if (v->u.a.capacity > v->u.a.size)
    {
        v->u.a.capacity = v->u.a.size;
        v->u.a.e = (diana_value *)realloc(v->u.a.e, v->u.a.size * sizeof(diana_value));
    }
}

size_t diana_get_array_capacity(const diana_value *v)
{
    assert(v != NULL && v->type == DIANA_ARRAY);
    return v->u.a.capacity;
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

diana_value *diana_pushback_array_element(diana_value *v)
{
    assert(v != NULL && v->type == DIANA_ARRAY);
    if (v->u.a.size == v->u.a.capacity)
        diana_reserve_array(v, v->u.a.capacity == 0 ? 1 : v->u.a.capacity * 2);
    diana_init(&v->u.a.e[v->u.a.size]);
    return &v->u.a.e[v->u.a.size++];
}

void diana_popback_array_element(diana_value *v)
{
    assert(v != NULL && v->type == DIANA_ARRAY && v->u.a.size > 0);
    diana_free(&v->u.a.e[--v->u.a.size]);
}

diana_value *diana_insert_array_element(diana_value *v, size_t index)
{
    assert(v != NULL && v->type == DIANA_ARRAY && index <= v->u.a.size);

    if (v->u.a.size + 1 >= v->u.a.capacity) // 检测是否插入一个元素会导致超容
        diana_reserve_array(v, v->u.a.capacity * 2);

    if (index == v->u.a.size) // 插入末尾
        return diana_pushback_array_element(v);

    // 插入数组中间
    // 0 1 2 3 4
    // 0 1 2 5 3 4
    diana_value *temp = (diana_value *)malloc(sizeof(diana_value) * (v->u.a.size - index));
    memcpy(temp, v->u.a.e + index, sizeof(diana_value) * (v->u.a.size - index));
    memcpy(v->u.a.e + index + 1, temp, sizeof(diana_value) * (v->u.a.size - index));
    v->u.a.size++;
    diana_free(&v->u.a.e[index]);
    diana_init(&v->u.a.e[index]);
    free(temp);
    return &v->u.a.e[index];

    return NULL;
}

void diana_erase_array_element(diana_value *v, size_t index, size_t count)
{
    assert(v != NULL && v->type == DIANA_ARRAY && index + count <= v->u.a.size);
    size_t i, tmp;
    tmp = index;
    for (i = 0; i < count; i++)
    {
        diana_free(&v->u.a.e[tmp++]);
    }
    // Move
    if (index + count < v->u.a.size)
    {
        diana_value *temp = (diana_value *)malloc(sizeof(diana_value) * (v->u.a.size - index - count));
        memcpy(temp, v->u.a.e + index + count, sizeof(diana_value) * (v->u.a.size - index - count));
        memcpy(v->u.a.e + index, temp, sizeof(diana_value) * (v->u.a.size - index - count));
        free(temp);

        // TODO diana_init移动后多余的节点
        // 0 1 2 3 4 5
        // 0 1 4 5 E E
    }

    v->u.a.size -= count;
}
void diana_clear_array(diana_value *v)
{
    assert(v != NULL && v->type == DIANA_ARRAY);
    diana_erase_array_element(v, 0, v->u.a.size);
}

void diana_set_object(diana_value *v, size_t capacity)
{
    assert(v != NULL);
    diana_free(v);
    v->type = DIANA_OBJECT;
    v->u.o.size = 0;
    v->u.o.capacity = capacity;
    v->u.o.m = capacity > 0 ? (diana_member *)malloc(capacity * sizeof(diana_member)) : NULL;
}

size_t diana_get_object_size(const diana_value *v)
{
    assert(v != NULL && v->type == DIANA_OBJECT);
    return v->u.o.size;
}

size_t diana_get_object_capacity(const diana_value *v)
{
    assert(v != NULL && v->type == DIANA_OBJECT);
    return v->u.o.capacity;
}

void diana_reserve_object(diana_value *v, size_t capacity)
{
    assert(v != NULL && v->type == DIANA_OBJECT);
    /* TODO */
    if (v->u.o.capacity < capacity)
    {
        v->u.o.capacity = capacity;
        v->u.o.m = (diana_member *)realloc(v->u.o.m, capacity * sizeof(diana_member));
    }
}

void diana_shrink_object(diana_value *v)
{
    assert(v != NULL && v->type == DIANA_OBJECT);
    /* TODO */
    if (v->u.o.capacity > v->u.o.size)
    {
        v->u.o.capacity = v->u.o.size;
        v->u.o.m = (diana_member *)realloc(v->u.o.m, v->u.o.size * sizeof(diana_member));
    }
}

void diana_clear_object(diana_value *v)
{
    assert(v != NULL && v->type == DIANA_OBJECT);
    /* TODO */
    size_t i;
    for (i = 0; i < v->u.o.size; i++)
    {
        // key and value
        free(v->u.o.m[i].k);
        v->u.o.m[i].klen = 0;
        diana_free(&v->u.o.m[i].v);
    }
    v->u.o.size = 0;
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

size_t diana_find_object_index(const diana_value *v, const char *key, size_t klen)
{
    size_t i;
    assert(v != NULL && v->type == DIANA_OBJECT && key != NULL);
    for (i = 0; i < v->u.o.size; i++)
    {
        if (v->u.o.m[i].klen == klen && memcmp(v->u.o.m[i].k, key, klen) == 0)
            return i;
    }
    return DIANA_KEY_NOT_EXIST;
}

diana_value *diana_find_object_value(diana_value *v, const char *key, size_t klen)
{
    size_t index = diana_find_object_index(v, key, klen);
    return index != DIANA_KEY_NOT_EXIST ? &v->u.o.m[index].v : NULL;
}

diana_value *diana_set_object_value(diana_value *v, const char *key, size_t klen)
{
    assert(v != NULL && v->type == DIANA_OBJECT && key != NULL);
    /* TODO */
    // 先搜寻是否存在该键
    size_t index = diana_find_object_index(v, key, klen);
    if (index != DIANA_KEY_NOT_EXIST)
    {
        // 键存在
        // diana_copy(diana_get_object_value(v, index), value);
        return diana_get_object_value(v, index);
    }
    else
    {
        // 键不存在，插入新键
        printf("New key:%s, klen=%zu\n", key, klen);

        if (v->u.o.size == v->u.o.capacity) // 先判断是否需要扩容
            diana_reserve_object(v, v->u.o.capacity == 0 ? 1 : v->u.o.capacity * 2);
        // key and value
        memcpy(v->u.o.m[v->u.o.size].k = (char *)malloc(klen + 1), key, klen);
        v->u.o.m[v->u.o.size].k[klen] = '\0';
        v->u.o.m[v->u.o.size].klen = klen;

        diana_init(&v->u.o.m[v->u.o.size].v);

        return &v->u.o.m[v->u.o.size++].v;
    }
}

void diana_remove_object_value(diana_value *v, size_t index)
{
    assert(v != NULL && v->type == DIANA_OBJECT && index < v->u.o.size);
    /* TODO */

    // key and value
    free(v->u.o.m[index].k);
    v->u.o.m[index].klen = 0;
    diana_free(&v->u.o.m[index].v);

    // move
    // 0 1 2 3 4
    // 0 1 3 4
    size_t i, tmp;
    tmp = index;
    for (i = 0; i < v->u.o.size - index - 1; i++)
    {
        // key and value
        if (v->u.o.m[tmp].k == NULL)
            v->u.o.m[tmp].k = (char *)malloc(v->u.o.m[tmp + 1].klen);
        memcpy(v->u.o.m[tmp].k, v->u.o.m[tmp + 1].k, v->u.o.m[tmp + 1].klen);
        v->u.o.m[tmp].klen = v->u.o.m[tmp + 1].klen;
        diana_move(&v->u.o.m[tmp].v, &v->u.o.m[tmp + 1].v);
        tmp++;
    }

    v->u.o.size--;
}

int diana_is_equal(const diana_value *lhs, const diana_value *rhs)
{
    size_t i;
    assert(lhs != NULL && rhs != NULL);
    if (lhs->type != rhs->type)
        return 0;
    switch (lhs->type)
    {
    case DIANA_STRING:
        return lhs->u.s.len == rhs->u.s.len &&
               memcmp(lhs->u.s.s, rhs->u.s.s, lhs->u.s.len) == 0;
    case DIANA_NUMBER:
        return lhs->u.n == rhs->u.n;
    case DIANA_ARRAY:
        if (lhs->u.a.size != rhs->u.a.size)
            return 0;
        for (i = 0; i < lhs->u.a.size; i++)
            if (!diana_is_equal(&lhs->u.a.e[i], &rhs->u.a.e[i]))
                return 0;
        return 1;
    case DIANA_OBJECT:
        if (lhs->u.o.size != rhs->u.o.size)
            return 0;
        for (i = 0; i < lhs->u.a.size; i++)
        {
            size_t index = diana_find_object_index(rhs, lhs->u.o.m[i].k, lhs->u.o.m[i].klen);
            if (index == DIANA_KEY_NOT_EXIST)
                return 0;
            if (!diana_is_equal(&lhs->u.o.m[i].v, &rhs->u.o.m[index].v))
                return 0;
        }
        return 1;
    default:
        return 1;
    }
}

#define PUTS(c, s, len) memcpy(diana_context_push(c, len), s, len)

static void diana_stringify_string(diana_context *c, const char *s, size_t len)
{
    static const char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    size_t i, size;
    char *head, *p;
    assert(s != NULL);
    p = head = diana_context_push(c, size = len * 6 + 2);
    *p++ = '"';
    for (i = 0; i < len; i++)
    {
        unsigned char ch = (unsigned char)s[i];
        switch (ch)
        {
        case '\"':
            *p++ = '\\';
            *p++ = '\"';
            break;
        case '\\':
            *p++ = '\\';
            *p++ = '\\';
            break;
        case '\b':
            *p++ = '\\';
            *p++ = 'b';
            break;
        case '\f':
            *p++ = '\\';
            *p++ = 'f';
            break;
        case '\n':
            *p++ = '\\';
            *p++ = 'n';
            break;
        case '\r':
            *p++ = '\\';
            *p++ = 'r';
            break;
        case '\t':
            *p++ = '\\';
            *p++ = 't';
            break;
        default:
            if (ch < 0x20)
            {
                *p++ = '\\';
                *p++ = 'u';
                *p++ = '0';
                *p++ = '0';
                *p++ = hex_digits[ch >> 4];
                *p++ = hex_digits[ch & 15];
            }
            else
                *p++ = s[i];
        }
    }
    *p++ = '"';
    c->top -= size - (p - head);
}

static void diana_stringify_value(diana_context *c, const diana_value *v)
{
    size_t i;
    switch (v->type)
    {
    case DIANA_NULL:
        PUTS(c, "null", 4);
        break;
    case DIANA_FALSE:
        PUTS(c, "false", 5);
        break;
    case DIANA_TRUE:
        PUTS(c, "true", 4);
        break;
    case DIANA_NUMBER:
        c->top -= 32 - sprintf(diana_context_push(c, 32), "%.17g", v->u.n);
        break;
    case DIANA_STRING:
        diana_stringify_string(c, v->u.s.s, v->u.s.len);
        break;
    case DIANA_ARRAY:
        PUTC(c, '[');
        for (i = 0; i < v->u.a.size; i++)
        {
            if (i > 0)
                PUTC(c, ',');
            diana_stringify_value(c, &v->u.a.e[i]);
        }
        PUTC(c, ']');
        break;
    case DIANA_OBJECT:
        PUTC(c, '{');
        for (i = 0; i < v->u.o.size; i++)
        {
            if (i > 0)
                PUTC(c, ',');
            diana_stringify_string(c, v->u.o.m[i].k, v->u.o.m[i].klen);
            PUTC(c, ':');
            diana_stringify_value(c, &v->u.o.m[i].v);
        }
        PUTC(c, '}');
        break;
    default:
        assert(0 && "invalid type");
    }
}

/* 生成器 */
char *diana_stringify(const diana_value *v, size_t *length)
{
    diana_context c;
    assert(v != NULL);
    c.stack = (char *)malloc(c.size = DIANA_PARSE_STRINGIFY_INIT_SIZE);
    c.top = 0;
    diana_stringify_value(&c, v);
    if (length)
        *length = c.top;
    PUTC(&c, '\0');
    return c.stack;
}

void diana_copy(diana_value *dst, const diana_value *src)
{
    size_t i;
    assert(src != NULL && dst != NULL && src != dst);
    switch (src->type)
    {
    case DIANA_STRING:
        diana_set_string(dst, src->u.s.s, src->u.s.len);
        break;
    case DIANA_ARRAY:
        /*TODO*/
        // How to allocate N
        diana_free(dst);
        // dst = (diana_value *)malloc(sizeof(diana_value)); // 给数组对象分配一个新空间
        dst->type = DIANA_ARRAY; // 划分类型为数组
        dst->u.a.size = src->u.a.size;
        dst->u.a.e = (diana_value *)malloc(sizeof(diana_value) * src->u.a.size); // 为数组元素中的数据区划分相同大小的空间
        for (i = 0; i < src->u.a.size; i++)
        {
            diana_init(&dst->u.a.e[i]); // 初始化当前元素
            diana_copy(&dst->u.a.e[i], &src->u.a.e[i]);
        }
        break;
    case DIANA_OBJECT:
        /*TODO*/
        // How to allocate N
        diana_free(dst);
        // dst = (diana_value *)malloc(sizeof(diana_value)); // 给对象分配一个新空间
        dst->type = DIANA_OBJECT; // 划分类型为对象
        dst->u.o.size = src->u.o.size;
        dst->u.o.m = (diana_member *)malloc(sizeof(diana_member) * src->u.o.size); // 为对象元素中的数据区划分相同大小的空间
        for (i = 0; i < src->u.o.size; i++)
        {
            /* key and value */
            dst->u.o.m[i].k = (char *)malloc(src->u.o.m[i].klen); // 为key字符串分配空间
            memcpy(dst->u.o.m[i].k, src->u.o.m[i].k, src->u.o.m[i].klen);
            dst->u.o.m[i].klen = src->u.o.m[i].klen;
            diana_init(&dst->u.o.m[i].v); // 初始化当前元素
            diana_copy(&dst->u.o.m[i].v, &src->u.o.m[i].v);
        }
        break;
    default: /* NUMBER, TRUE, FALSE, NULL */
        diana_free(dst);
        memcpy(dst, src, sizeof(diana_value));
        break;
    }
}
#define NAME(x) #x

void diana_move(diana_value *dst, diana_value *src)
{
    if (dst == NULL)
        printf("dst is null!\n");
    if (src == NULL)
        printf("src is null!\n");
    if (src == dst)
        printf("src==dst!\n");
    assert(dst != NULL && src != NULL && src != dst);
    diana_free(dst);
    memcpy(dst, src, sizeof(diana_value));
    diana_init(src);
}

void diana_swap(diana_value *lhs, diana_value *rhs)
{
    assert(lhs != NULL && rhs != NULL);
    if (lhs != rhs)
    {
        diana_value temp;
        memcpy(&temp, lhs, sizeof(diana_value));
        memcpy(lhs, rhs, sizeof(diana_value));
        memcpy(rhs, &temp, sizeof(diana_value));
    }
}