/* 简易单元测试 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dianajson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format)                                                           \
    do                                                                                                             \
    {                                                                                                              \
        test_count++;                                                                                              \
        if (equality)                                                                                              \
            test_pass++;                                                                                           \
        else                                                                                                       \
        {                                                                                                          \
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual); \
            main_ret = 1;                                                                                          \
        }                                                                                                          \
    } while (0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

static void test_parse_null()
{
    diana_value v;
    v.type = DIANA_FALSE;
    EXPECT_EQ_INT(DIANA_PARSE_OK, diana_parse(&v, "null"));
    EXPECT_EQ_INT(DIANA_NULL, diana_get_type(&v));
}

static void test_parse_expect_value()
{
    diana_value v;

    v.type = DIANA_FALSE;
    EXPECT_EQ_INT(DIANA_PARSE_EXPECT_VALUE, diana_parse(&v, ""));
    EXPECT_EQ_INT(DIANA_NULL, diana_get_type(&v));

    v.type = DIANA_FALSE;
    EXPECT_EQ_INT(DIANA_PARSE_EXPECT_VALUE, diana_parse(&v, " "));
    EXPECT_EQ_INT(DIANA_NULL, diana_get_type(&v));
}

static void test_parse_invalid_value()
{
    diana_value v;
    v.type = DIANA_FALSE;
    EXPECT_EQ_INT(DIANA_PARSE_INVALID_VALUE, diana_parse(&v, "nul"));
    EXPECT_EQ_INT(DIANA_NULL, diana_get_type(&v));

    v.type = DIANA_FALSE;
    EXPECT_EQ_INT(DIANA_PARSE_INVALID_VALUE, diana_parse(&v, "?"));
    EXPECT_EQ_INT(DIANA_NULL, diana_get_type(&v));
}

static void test_parse_root_not_singular()
{
    diana_value v;
    v.type = DIANA_FALSE;
    EXPECT_EQ_INT(DIANA_PARSE_ROOT_NOT_SINGULAR, diana_parse(&v, "null x"));
    EXPECT_EQ_INT(DIANA_NULL, diana_get_type(&v));
}

static void test_parse()
{
    test_parse_null();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
}

int main()
{
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);

    return main_ret;
}