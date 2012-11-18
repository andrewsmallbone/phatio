
// hideous - redo all of this

#include "test.h"
#include "lio.h"
#include "lio_builtins.h"
#include "char_source.h"
#include <string.h>
#include <util.h>

void *get_variable_ref(Item *pointer_or_name) { return 0;}
Item *get_variable_item(Item *item){ return 0;}
void *resolve_reference(const char *name, lio_type *type) {
    if (name[0] == 'r') {
        *type = LIO_FUNCTION;
        return (void *)9999;
    } else {
        return 0;
    }
}

// needed for linking but not called/tested
builtin_handler *find_handler(const char *name) {return 0;}

void lio_stop(const char *message) {}
void *eval_as_reference(Item *fp_or_name) { return 0;}

#define SIZE 255
uint8_t temp[64];


void print(List *cl)
{
    printf("\n");
    for (int i=0; i<200; i++) {
        printf("%u ", *(((char *)cl)+i));
    }
    printf("\n");
}


void test_next_expression()
{
    uint8_t buf[SIZE];
    memset(buf, 0, SIZE);
    char_source *src = string_source("(ra \"bb\" \"cc\" # \n \"dd\") ");
    List *cl = next_expression(src, buf, SIZE);

    assert_equals(19, list_size(cl));
    assert_equals(3, item_size(first(cl)));
    assert_equals(15, list_size(rest(cl)));
    assert_equals(4, item_size(second(cl)));
    assert_equals(10, list_size(rest(rest(cl))));
    assert_equals(4, item_size(third(cl)));
    assert_equals(5, list_size(rest(rest(rest(cl)))));
    assert_equals(4, item_size(fourth(cl)));
    assert_equals(0, list_size(rest(rest(rest(rest(cl))))));

    // aa
    assert_equals(LIO_FUNCTION, first(cl)->type);

    // bb
    assert_equals(LIO_STRING, first(rest(cl))->type);
    assert_string_equals("bb", eval_as_string(first(rest(cl))));
    assert_equals(LIO_STRING, second(cl)->type);
    assert_string_equals("bb", eval_as_string(second(cl)));

    assert_equals(LIO_STRING, first(rest(rest(cl)))->type);
    assert_string_equals("cc", eval_as_string(first(rest(rest(cl)))));
    assert_equals(LIO_STRING, third(cl)->type);
    assert_string_equals("cc", eval_as_string(third(cl)));

    assert_equals(LIO_STRING, first(rest(rest(rest(cl))))->type);
    assert_string_equals("dd", eval_as_string(first(rest(rest(rest(cl))))));
    assert_equals(LIO_STRING, fourth(cl)->type);
    assert_string_equals("dd", eval_as_string(fourth(cl)));

    assert_equals(0, rest(rest(rest(rest(cl)))));
    assert_equals(0, first(rest(rest(rest(rest(cl))))));
}

void test_next_expression_inner_list()
{
    uint8_t buf[SIZE];
    memset(buf, 0, SIZE);
    char_source *src = string_source("(ra (rb) \"cc\") ");
    List *cl = next_expression(src, buf, SIZE);

    // 'aa'
    assert_equals(LIO_FUNCTION, first(cl)->type);

    // (bb)
    assert_equals(LIO_LIST, first(rest(cl))->type);
    List *second = as_list(first(rest(cl)));

    assert_equals(LIO_FUNCTION, first(second)->type);

    assert_equals(0, rest(second));
    assert_equals(0, rest(rest(rest(second))));

    //cc
    assert_equals(LIO_STRING, third(cl)->type);
    assert_string_equals("cc", eval_as_string(third(cl)));
    assert_equals(0, fourth(cl));
}

void test_next_expression_hierarchical_list()
{
    uint8_t buf[SIZE];
    memset(buf, 0, SIZE);

    char_source *src = string_source("(ra (rb \"cc\") (rd))");
    List *cl = next_expression(src, buf, SIZE);
    assert_equals(21, list_size(cl));
    assert_equals(17, list_size(rest(cl)));
    assert_equals(6, list_size(rest(rest(cl))));


    assert_equals(LIO_FUNCTION, item_type(first(cl)));
    assert_equals(3, item_size(first(cl)));

    // (bb cc)
    assert_equals(LIO_LIST, second(cl)->type);

    assert_equals(9, list_size(as_list(second(cl))));
    assert_equals(5, list_size(rest(as_list(second(cl)))));
    assert_equals(0, list_size(rest(rest(as_list(second(cl))))));

    List *l = as_list(second(cl));
    assert_equals(3, item_size(first(l)));
    assert_equals(LIO_FUNCTION, first(l)->type);

    assert_equals(LIO_STRING, second(l)->type);
    assert_equals(4, item_size(second(l)));
    assert_string_equals("cc", eval_as_string(second(l)));

    assert_equals(0, third(l));
    assert_equals(0, item_size(third(l)));

    // (dd)
    assert_equals(LIO_LIST, third(cl)->type);
    assert_equals(LIO_FUNCTION, first(as_list(third(cl)))->type);

    assert_equals(5, item_size(third(cl)));
    assert_equals(3, item_size(first(as_list(third(cl)))));
}


void test_empty_expression()
{
    uint8_t buf[SIZE];
    memset(buf, 0, SIZE);

    char_source *src = string_source(" ");
    List *cl = next_expression(src, buf, SIZE);

    assert_equals(0, first(cl));
    assert_equals(0, rest(cl));
}

void test_complex()
{
    uint8_t buf[SIZE];
    memset(buf, 0, SIZE);
    char_source *src = string_source("(rspi_conf 0 msb 6) (rsetpin b6 0)");
    List *exp = next_expression(src, buf, SIZE);


    // first expressions
    assert_equals(LIO_FUNCTION, item_type(first(exp)));

    exp = rest(exp);
    assert_equals(LIO_UINT8, item_type(first(exp)));
    assert_equals(0, eval_as_uint8(first(exp)));

    exp = rest(exp);
    assert_equals(LIO_ATOM, item_type(first(exp)));
    assert_string_equals("msb", eval_as_string(first(exp)));

    exp = rest(exp);
    assert_equals(LIO_UINT8, item_type(first(exp)));
    assert_equals(6, eval_as_uint8(first(exp)));

    assert_equals(NULL, rest(exp));

    // second expression
    exp = next_expression(src, buf, SIZE);
    assert_equals(LIO_FUNCTION, item_type(first(exp)));

    exp = rest(exp);
    assert_equals(LIO_ATOM, item_type(first(exp)));
    assert_string_equals("b6", eval_as_string(first(exp)));

    exp = rest(exp);
    assert_equals(LIO_UINT8, item_type(first(exp)));
    assert_equals(0, eval_as_uint8(first(exp)));

    assert_equals(NULL, rest(exp));
    assert_equals(NULL, next_expression(src, buf, SIZE));
}


void test_complex2()
{
    uint8_t buf[SIZE];
    memset(buf, 0, SIZE);
    char_source *src = string_source("(rdriver mux (rsetpin 13 0) (rkeyboard (read_uint8 0) (read_uint8 2)) (rsetpin 13 1) )");
//    char_source *src = string_source("(((a) b))");
    List *exp = next_expression(src, buf, SIZE);

    assert_equals(LIO_FUNCTION, item_type(first(exp)));
    assert_string_equals("mux", eval_as_string(second(exp)));

    List *one = as_list(third(exp));
    assert_equals(LIO_FUNCTION, item_type(first(one)));
    assert_equals(13, eval_as_uint8(second(one)));
    assert_equals(0, eval_as_uint8(third(one)));
    assert_equals(0, fourth(one));

    List *two = as_list(fourth(exp));
    assert_equals(LIO_FUNCTION, item_type(first(two)));

    assert_equals(LIO_FUNCTION, item_type(first(as_list(second(two)))));
    assert_string_equals("0", eval_as_string(second(as_list(second(two)))));
    assert_equals(0, rest(rest((as_list(second(two))))));

    assert_equals(LIO_FUNCTION, item_type(first(as_list(third(two)))));
    assert_string_equals("2", eval_as_string(second(as_list(third(two)))));
    assert_equals(0, rest(rest((as_list(third(two))))));

    assert_equals(0, fourth(two));

    List *three = as_list(first(rest(rest(rest(rest(exp))))));
    assert_equals(LIO_FUNCTION, item_type(first(three)));
    assert_string_equals("13", eval_as_string(second(three)));
    assert_string_equals("1", eval_as_string(third(three)));
    assert_equals(0, fourth(three));

}


void test_complex3()
{
    uint8_t buf[SIZE];
    memset(buf, 0, SIZE);
    char_source *src = string_source("(ra ((rb)) ee )");
    List *exp = next_expression(src, buf, SIZE);
    assert_equals(LIO_FUNCTION, item_type(first(exp)));

    List *bb = as_list(first(as_list(second(exp))));
    assert_equals(LIO_FUNCTION, item_type(first(bb)));

    Item *ee = third(exp);
    assert_string_equals("ee", eval_as_string(ee));

}

void test_large()
{
//    uint8_t buf[SIZE];
//    memset(buf, 0, SIZE);
//#define INNER "(reyboard (r 2 (r 2 (r 2 (r 2 (r 2 3)))) ) )"
//
//    char_source *src = string_source("(rriver aab "   INNER INNER INNER ")");
//    List *expression = next_expression(src, buf, SIZE);
//
//    int i=0;
//    printf("#%u\n", list_size(expression));
//    for (List *remaining = expression; remaining; remaining = rest(remaining)) {
//        printf("%d:%u/%u/%u/%u\n", i, item_type(first(remaining)), list_size(as_list(first(remaining))), item_size(first(remaining)), list_size(rest(remaining)));
//        i++;
//    }

}

void check_tokenizer(char *input, char *expected[], int expected_len)
{
    char_source *src = string_source(input);

    for (int i=0; i<expected_len; i++) {
        assert_equals(true, next_token(src, temp, 64));
        assert_string_equals(expected[i], temp);
    }

    assert_equals(false, next_token(src, temp, 64));
}




void test_tokenization()
{
    char *expected[] = {"(", "conditions", ")"};
    check_tokenizer(" ( conditions ) ", expected, 3);
    check_tokenizer("(conditions)", expected, 3);

    char *expected1[] = {"(", "conditions", "(", "aa", "bb", "cc", ")", "(", "xx", "yy", "zz", ")", ")"};
    check_tokenizer("(conditions (aa bb cc ) (xx yy zz) ) ", expected1, 13);

    char *expected2[] = {"(", "conditions", "(", "aa", "\"this is a string\"", "cc", ")", "(", "xx", "yy", "zz", ")", ")"};
    check_tokenizer("(conditions (aa \"this is a string\" cc ) (xx yy zz)) ", expected2, 13);

    char *expected3[] = {"(", "str-test", "\"thi((\\\")\"", ")"};
    check_tokenizer("(str-test \"thi((\\\")\") ", expected3, 4);

    char *expected4[] = {"(", "conditions", "(", "keyboard", "hello", ")", ")"};
    check_tokenizer(" ( conditions (keyboard hello)) ", expected4, 7);

    char *expected5[] = {"(", "driver", "mux", "(", "setpin", "13", "0", ")", "(", "keyboard", "(",
            "read_uint8", "0", ")", "(", "read_uint8", "2", ")", ")", "(", "setpin", "13", "1", ")", ")"};
    check_tokenizer("(driver mux (setpin 13 0) (keyboard (read_uint8 0) (read_uint8 2)) (setpin 13 1) )", expected5, 25);

}



void test_lio(void)
{
    test_tokenization();
    test_next_expression();
    test_next_expression_inner_list();
    test_next_expression_hierarchical_list();
    test_empty_expression();
    test_complex();
    test_complex2();
    test_complex3();
    test_large();
}
