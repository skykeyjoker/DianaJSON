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
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

#define TEST_NUMBER(expect, json)                             \
    do                                                        \
    {                                                         \
        diana_value v;                                        \
        EXPECT_EQ_INT(DIANA_PARSE_OK, diana_parse(&v, json)); \
        EXPECT_EQ_INT(DIANA_NUMBER, diana_get_type(&v));      \
        EXPECT_EQ_DOUBLE(expect, diana_get_number(&v));       \
    } while (0)

#define TEST_ERROR(error, json)                        \
    do                                                 \
    {                                                  \
        diana_value v;                                 \
        v.type = DIANA_FALSE;                          \
        EXPECT_EQ_INT(error, diana_parse(&v, json));   \
        EXPECT_EQ_INT(DIANA_NULL, diana_get_type(&v)); \
    } while (0)

static void test_parse_number()
{
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002");           /* the smallest number > 1 */
    TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308"); /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308"); /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308"); /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_parse_null()
{
    diana_value v;
    v.type = DIANA_FALSE;
    EXPECT_EQ_INT(DIANA_PARSE_OK, diana_parse(&v, "null"));
    EXPECT_EQ_INT(DIANA_NULL, diana_get_type(&v));
}

static void test_parse_true()
{
    diana_value v;
    v.type = DIANA_FALSE;
    EXPECT_EQ_INT(DIANA_PARSE_OK, diana_parse(&v, "true"));
    EXPECT_EQ_INT(DIANA_TRUE, diana_get_type(&v));
}

static void test_parse_false()
{
    diana_value v;
    v.type = DIANA_TRUE;
    EXPECT_EQ_INT(DIANA_PARSE_OK, diana_parse(&v, "false"));
    EXPECT_EQ_INT(DIANA_FALSE, diana_get_type(&v));
}

static void test_parse_expect_value()
{
    TEST_ERROR(DIANA_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(DIANA_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value()
{
    TEST_ERROR(DIANA_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(DIANA_PARSE_INVALID_VALUE, "?");

    /* invalid number */
    TEST_ERROR(DIANA_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(DIANA_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(DIANA_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(DIANA_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(DIANA_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(DIANA_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(DIANA_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(DIANA_PARSE_INVALID_VALUE, "nan");
}

static void test_parse_root_not_singular()
{
    TEST_ERROR(DIANA_PARSE_ROOT_NOT_SINGULAR, "null x");

    /* invalid number */
    TEST_ERROR(DIANA_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
    TEST_ERROR(DIANA_PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(DIANA_PARSE_ROOT_NOT_SINGULAR, "0x123");
}

static void test_parse_number_too_big()
{
    TEST_ERROR(DIANA_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(DIANA_PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void test_parse()
{
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();
}

int main()
{
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}