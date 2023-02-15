#include "grammar.h"
#include "test.h"

#define TEST_GRAMMAR(path, pass) {\
    ParserState state = {0}; \
    AstNode* root = NULL; \
    soc_context_t *parser = NULL; \
    if (setjmp(state.jmp) == 0) { \
        ParserState_Init(&state); \
        ParserState_Open(&state, (path)); \
        parser = soc_create(&state); \
        const int b = soc_parse(parser, &root); \
        ParserState_Dump(&state, root, stdin); \
        if ((pass)) { \
            TEST_ASSERT(b == 1); \
        } else { \
            TEST_ASSERT(b == 0); \
        } \
    } else {\
        TEST_ASSERT((pass) == 0); \
    }\
    soc_destroy(parser); \
    ParserState_Finalize(&state); \
}

int main(int argc, char **argv) {
    TEST_BEGIN(("grammar_test"));
    TEST_GRAMMAR("src/tests/y_int_lit_100_000_000.txt", 1);
    TEST_GRAMMAR("src/tests/y_hex_lit_0x8000_ff00.txt", 1);
    TEST_GRAMMAR("src/tests/n_hex_lit_0xz.txt", 0);
    TEST_END();
    return 0;
}
