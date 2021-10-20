#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dianajson.h"

int main()
{
    diana_value v1;
    diana_init(&v1);
    diana_parse(&v1, "{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":\"3\"}}");
    printf("Json Size:%zu, Capacity:%zu\n", v1.u.o.size, v1.u.o.capacity);
    if (diana_get_object_value(&v1, diana_find_object_index(&v1, "n", 1))->type == DIANA_NULL)
        printf("First value is NULL\n");
    if (diana_get_object_value(&v1, diana_find_object_index(&v1, "f", 1))->type == DIANA_FALSE)
        printf("Second value is FALSE\n");
    if (diana_get_object_value(&v1, diana_find_object_index(&v1, "t", 1))->type == DIANA_TRUE)
        printf("Third value is TRUE\n");
    if (diana_get_object_value(&v1, diana_find_object_index(&v1, "i", 1))->type == DIANA_NUMBER)
    {
        printf("Forth value is NUMBER:%f\n", diana_find_object_value(&v1, "i", 1)->u.n);
    }
    if (diana_get_object_value(&v1, diana_find_object_index(&v1, "s", 1))->type == DIANA_STRING)
    {
        printf("Fifth value is STRING:%s, LEN:%zu\n", diana_find_object_value(&v1, "s", 1)->u.s.s, diana_find_object_value(&v1, "s", 1)->u.s.len);
    }
    if (diana_get_object_value(&v1, diana_find_object_index(&v1, "a", 1))->type == DIANA_ARRAY)
    {
        printf("Sixth value is ARRAY, SIZE:%zu; ", diana_find_object_value(&v1, "a", 1)->u.a.size);
        printf("[");
        for (size_t i = 0; i < diana_find_object_value(&v1, "a", 1)->u.a.size; i++)
        {
            printf("%f,", diana_find_object_value(&v1, "a", 1)->u.a.e[i].u.n);
        }
        printf("]\n");
    }
    if (diana_get_object_value(&v1, diana_find_object_index(&v1, "o", 1))->type == DIANA_OBJECT)
    {
        printf("Seventh value is OBJECT, SIZE:%zu\n", diana_find_object_value(&v1, "s", 1)->u.o.size);
        printf("{");
        for (size_t i = 0; i < diana_find_object_value(&v1, "o", 1)->u.o.size - 1; i++)
        {
            printf("%s:%f,", diana_find_object_value(&v1, "o", 1)->u.o.m[i].k, diana_find_object_value(&v1, "o", 1)->u.o.m[i].v.u.n);
        }
        printf("%s:%s,", diana_find_object_value(&v1, "o", 1)->u.o.m[diana_find_object_value(&v1, "o", 1)->u.o.size - 1].k, diana_find_object_value(&v1, "o", 1)->u.o.m[diana_find_object_value(&v1, "o", 1)->u.o.size - 1].v.u.s.s);
        printf("}\n");
    }

    char *json_str;
    json_str = diana_stringify(&v1, NULL);
    printf("JSON:%s\n", json_str);

    return 0;
}