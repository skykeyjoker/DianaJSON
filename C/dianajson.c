#include "dianajson.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h> /* errno, ERANGE */
#include <math.h>  /* HUGE_VAL */

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
} diana_context;

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
    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
        return DIANA_PARSE_NUMBER_TOO_BIG;
    v->type = DIANA_NUMBER;
    c->json = p;
    return DIANA_PARSE_OK;
}

/* value = null / false / true / number */
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
    v->type = DIANA_NULL;
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
    return ret;
}

diana_type diana_get_type(const diana_value *v)
{
    assert(v != NULL);
    return v->type;
}

double diana_get_number(const diana_value *v)
{
    assert(v != NULL && v->type == DIANA_NUMBER);
    return v->n;
}
