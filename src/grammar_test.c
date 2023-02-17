#include "grammar.h"
#include "test.h"

#define TEST_GRAMMAR(path, pass) {\
    ParserState state = {0}; \
    int ret = 0; \
    soc_context_t *parser = NULL; \
    if (setjmp(state.jmp) == 0) { \
        ParserState_Init(&state); \
        ParserState_Open(&state, (path)); \
        parser = soc_create(&state); \
        const int b = soc_parse(parser, &ret); \
        TEST_ASSERT(ret == 0); \
        TEST_ASSERT(b == 0); \
    } else {\
        TEST_ASSERT((pass) == 0); \
    }\
    soc_destroy(parser); \
    ParserState_Finalize(&state); \
}

int main(int argc, char **argv) {
    TEST_BEGIN(("grammar_test"));
    TEST_GRAMMAR("src/tests/good.txt", 1);
    TEST_END();
    return 0;
}
