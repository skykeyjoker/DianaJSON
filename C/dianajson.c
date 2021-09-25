#include "dianajson.h"
#include <assert.h>
#include <stddef.h>

#define EXPECT(c, ch)             \
    do                            \
    {                             \
        assert(*c->json == (ch)); \
        c->json++;                \
    } while (0)

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

/* 解析null */
/* null = "null" */
static int diana_parse_null(diana_context *c, diana_value *v)
{
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return DIANA_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = DIANA_NULL;
    return DIANA_PARSE_OK;
}

/* 解析true */
/* true = "true" */
static int diana_parse_true(diana_context *c, diana_value *v)
{
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return DIANA_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = DIANA_TRUE;
    return DIANA_PARSE_OK;
}

/* 解析false */
/* false = "false" */
static int diana_parse_false(diana_context *c, diana_value *v)
{
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return DIANA_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = DIANA_FALSE;
    return DIANA_PARSE_OK;
}

/* value = null / false / true */
static int diana_parse_value(diana_context *c, diana_value *v)
{
    switch (*c->json)
    {
    case 't':
        return diana_parse_true(c, v);
    case 'f':
        return diana_parse_false(c, v);
    case 'n':
        return diana_parse_null(c, v);
    case '\0':
        return DIANA_PARSE_EXPECT_VALUE;
    default:
        return DIANA_PARSE_INVALID_VALUE;
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
            ret = DIANA_PARSE_ROOT_NOT_SINGULAR;
    }
    return ret;
}

diana_type diana_get_type(const diana_value *v)
{
    assert(v != NULL);
    return v->type;
}
