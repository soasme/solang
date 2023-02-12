#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "lexer.h"

#define LEXER_TEST_PASS(input, expected, expected_pos) {\
    LexerState s = {\
        .source = (input), \
        .line = 1, \
        .column = 1, \
        .error = LEXER_EOK, \
    }; \
    s.current = s.source; \
    assert(next_token(&s) == (expected)); \
    assert(s.current == s.source + (expected_pos));\
}

#define LEXER_TEST_FAILED(input, expected_error, expected_pos) {\
    LexerState s = {\
        .source = (input), \
        .line = 1, \
        .column = 1, \
        .error = LEXER_EOK, \
    }; \
    s.current = s.source; \
    assert(next_token(&s) == TOKEN_ERROR); \
    assert(s.error == (expected_error)); \
    assert(s.current == s.source + (expected_pos));\
}

int main(int argc, char **argv) {

    LEXER_TEST_PASS("", TOKEN_EOF, 0);
    LEXER_TEST_PASS(" \t\r\n", TOKEN_EOF, 4);
    LEXER_TEST_PASS("# comment", TOKEN_EOF, 9);
    LEXER_TEST_PASS("# comment\n", TOKEN_EOF, 10);
    LEXER_TEST_PASS("42", TOKEN_INT_LITERAL, 2);
    LEXER_TEST_PASS("424_242_424", TOKEN_INT_LITERAL, 11);
    LEXER_TEST_PASS("42   ", TOKEN_INT_LITERAL, 5);
    LEXER_TEST_PASS("42.0", TOKEN_FLOAT_LITERAL, 4);
    LEXER_TEST_PASS("42.000", TOKEN_FLOAT_LITERAL, 6);
    LEXER_TEST_PASS("42.000   ", TOKEN_FLOAT_LITERAL, 9);
    LEXER_TEST_PASS("42.0e42", TOKEN_FLOAT_LITERAL, 7);
    LEXER_TEST_PASS("42.0e+42", TOKEN_FLOAT_LITERAL, 8);
    LEXER_TEST_PASS("42.0e-42", TOKEN_FLOAT_LITERAL, 8);
    LEXER_TEST_PASS("42.0E+42", TOKEN_FLOAT_LITERAL, 8);
    LEXER_TEST_PASS("42.0E-42", TOKEN_FLOAT_LITERAL, 8);
    LEXER_TEST_PASS("0b01001", TOKEN_INT_LITERAL, 7);
    LEXER_TEST_FAILED("0b010012", LEXER_EBINCHR, 7);
    LEXER_TEST_FAILED("0b201001", LEXER_EBINCHR, 2);
    LEXER_TEST_PASS("0o777", TOKEN_INT_LITERAL, 5);
    LEXER_TEST_FAILED("0o7778", LEXER_EOCTCHR, 5);
    LEXER_TEST_PASS("0x000A", TOKEN_INT_LITERAL, 6);
    LEXER_TEST_FAILED("\"\n\"", LEXER_EMULTILINESTR, 1);
    LEXER_TEST_PASS("\"\\n\"", TOKEN_STR_LITERAL, 4);
    LEXER_TEST_PASS("\"\\U00a000a0\"", TOKEN_STR_LITERAL, 12);
    LEXER_TEST_FAILED("\"\\U000a\"", LEXER_EUTF8UNDER8, 7);
    LEXER_TEST_PASS("\"\\u00a0\"", TOKEN_STR_LITERAL, 8);
    LEXER_TEST_PASS("\"\\u00A0\"", TOKEN_STR_LITERAL, 8);
    LEXER_TEST_FAILED("\"\\u0a\"", LEXER_EUTF8UNDER4, 5);
    LEXER_TEST_PASS("\"你好世界\"", TOKEN_STR_LITERAL, 14);
    LEXER_TEST_FAILED("@", LEXER_EINVALIDCHAR, 0);
    LEXER_TEST_PASS("'a'", TOKEN_CHAR_LITERAL, 3);
    LEXER_TEST_PASS("'你'", TOKEN_CHAR_LITERAL, 5);
    LEXER_TEST_PASS("'\\n'", TOKEN_CHAR_LITERAL, 4);
    LEXER_TEST_PASS("'\\xA0'", TOKEN_CHAR_LITERAL, 6);
    LEXER_TEST_PASS("'\\u00A0'", TOKEN_CHAR_LITERAL, 8);
    LEXER_TEST_PASS("'\\U00A000A0'", TOKEN_CHAR_LITERAL, 12);
    LEXER_TEST_FAILED("''", LEXER_EEMPTYCHR, 1);

    printf("all tests passed!\n");

    return 0;
}
