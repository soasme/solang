#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "lexer.h"

// get_tok_name returns the name of the given token.
static inline char* get_tok_name(TokenKind token) {
    return TokenName[token];
}

// to_string returns a string representation of the given parser state.
// This is used for debugging.
static char* to_string(LexerState* state) {
    char* str = malloc(100);
    memset(str, 0, 100);
    sprintf(str, "line %d, col %d", state->line, state->column);
    return str;
}

// get_chr returns the character at the given offset from the current position.
// This function does not check for EOF.
static inline char get_chr(LexerState* state, size_t offset) {
    return *(state->current + offset);
}

// skip_line_comment skips a line comment.
static void skip_line_comment(LexerState* state) {
    char c;
    do {
        c = *state->current++;
        state->column++;
    } while (c != '\n' && c != '\0');
    if (c == '\n') {
        state->line++;
        state->column = 1;
    } else {
        state->current--;
    }
}

// is_skip returns true if the given character is whitespace, tab, newline,
static bool is_skip(LexerState* state) {
    char c = get_chr(state, 0);
    switch (c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        case '#':
            return true;
        default:
            return false;
    }
}

// skip skips whitespace, tab, newlines (clrf), line comments.
static void skip(LexerState* state) {
    while (true) {
        char c = get_chr(state, 0);
        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                state->current++;
                state->column++;
                break;
            case '\n':
                state->current++;
                state->line++;
                state->column = 1;
                break;
            case '#':
                skip_line_comment(state);
                break;
            default:
                return;
        }
    }
}

// next_bin returns the next binary number.
static TokenKind next_bin_int(LexerState* state) {
    char c = get_chr(state, 0);

    // [01]
    if (c == '0' || c == '1') {
        state->current++;
        state->column++;
    } else {
        state->error = LEXER_EBINCHR;
        return TOKEN_ERROR;
    }

    // ('_'? [01])*
    while (true) {
        c = get_chr(state, 0);
        if (c == '_') {
            state->current++;
            state->column++;
            c = get_chr(state, 0);
        }
        if (c == '0' || c == '1') {
            state->current++;
            state->column++;
        } else {
            break;
        }
    }

    // ![23456789]
    c = get_chr(state, 0);
    if (c >= '2' && c <= '9') {
        state->error = LEXER_EBINCHR;
        return TOKEN_ERROR;
    }

    return TOKEN_INT_LITERAL;
}

// next_oct returns the next octal number.
static TokenKind next_oct_int(LexerState* state) {
    char c = get_chr(state, 0);

    // [0-7]
    if (c >= '0' || c <= '7') {
        state->current++;
        state->column++;
    } else {
        state->error = LEXER_EOCTCHR;
        return TOKEN_ERROR;
    }

    // ('_'? [0-7])*
    while (true) {
        c = get_chr(state, 0);
        if (c == '_') {
            state->current++;
            state->column++;
            c = get_chr(state, 0);
        }
        if (c >= '0' && c <= '7') {
            state->current++;
            state->column++;
        } else {
            break;
        }
    }

    // ![89]
    c = get_chr(state, 0);
    if (c >= '8' && c <= '9') {
        state->error = LEXER_EOCTCHR;
        return TOKEN_ERROR;
    }

    return TOKEN_INT_LITERAL;
}

// next_hex_int returns the next hex integer.
static TokenKind next_hex_int(LexerState* state) {
    char c = get_chr(state, 0);

    // [0-9a-fA-F]
    if (c >= '0' || c <= '9' || c >= 'a' || c <= 'f' || c >= 'A' || c <= 'F') {
        state->current++;
        state->column++;
    } else {
        state->error = LEXER_EHEXCHR;
        return TOKEN_ERROR;
    }

    // ('_'? [0-9a-fA-F])*
    while (true) {
        c = get_chr(state, 0);
        if (c == '_') {
            state->current++;
            state->column++;
            c = get_chr(state, 0);
        }
        if (c >= '0' && c <= '9' || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F') {
            state->current++;
            state->column++;
        } else {
            break;
        }
    }

    return TOKEN_INT_LITERAL;
}

// next_hex returns the next hex number.
static TokenKind next_hex(LexerState* state) {
    TokenKind token = next_hex_int(state);
    if (token != TOKEN_INT_LITERAL) {
        return token;
    }

    bool dot = false;
    bool sci_note = false;

    char c = get_chr(state, 0);

    if (c == '.') {
        dot = true;
        state->current++;
        state->column++;

        token = next_hex_int(state);
        if (token != TOKEN_INT_LITERAL) {
            return token;
        }
    }

    c = get_chr(state, 0);

    if (c == 'p' || c == 'P') {
        sci_note = true;
        state->current++;
        state->column++;

        c = get_chr(state, 0);
        if (c == '+' || c == '-') {
            state->current++;
            state->column++;
        }

        token = next_hex_int(state);
        if (token != TOKEN_INT_LITERAL) {
            return token;
        }
    }

    return dot || sci_note ? TOKEN_FLOAT_LITERAL : TOKEN_INT_LITERAL;
}

// next_dec_int returns the next decimal integer.
static TokenKind next_dec_int(LexerState* state) {
    char c = get_chr(state, 0);

    // [0-9]
    if (c >= '0' || c <= '9') {
        state->current++;
        state->column++;
    } else {
        return TOKEN_ERROR;
    }

    // ('_'? [0-9])*
    while (true) {
        c = get_chr(state, 0);
        if (c == '_') {
            state->current++;
            state->column++;
            c = get_chr(state, 0);
        }
        if (c >= '0' && c <= '9') {
            state->current++;
            state->column++;
        } else {
            break;
        }
    }

    return TOKEN_INT_LITERAL;
}

// next_dec returns the next decimal number.
static TokenKind next_dec(LexerState* state) {
    TokenKind token = next_dec_int(state);
    if (token != TOKEN_INT_LITERAL) {
        return token;
    }

    bool dot = false;
    bool sci_note = false;

    char c = get_chr(state, 0);

    if (c == '.') {
        dot = true;
        state->current++;
        state->column++;

        token = next_dec_int(state);
        if (token != TOKEN_INT_LITERAL) {
            return token;
        }
    }

    c = get_chr(state, 0);

    if (c == 'e' || c == 'E') {
        sci_note = true;
        state->current++;
        state->column++;

        c = get_chr(state, 0);
        if (c == '+' || c == '-') {
            state->current++;
            state->column++;
        }

        token = next_dec_int(state);
        if (token != TOKEN_INT_LITERAL) {
            return token;
        }
    }

    return dot || sci_note ? TOKEN_FLOAT_LITERAL : TOKEN_INT_LITERAL;
}



// next_num returns the next int or float token in the given parser state.
static TokenKind next_num(LexerState* state) {
    char c0 = get_chr(state, 0);
    char c1 = get_chr(state, 1);
    TokenKind token;
    if (c0 == '0') {
        switch (c1) {
            case 'b':
            case 'B':
                state->current += 2;
                state->column += 2;
                token = next_bin_int(state);
                skip(state);
                return token;
            case 'o':
            case 'O':
                state->current += 2;
                state->column += 2;
                token = next_oct_int(state);
                skip(state);
                return token;
            case 'x':
            case 'X':
                state->current += 2;
                state->column += 2;
                token = next_hex(state);
                skip(state);
                return token;
            default:
                token = next_dec(state);
                skip(state);
                return token;
        }
    } else {
        token = next_dec(state);
        skip(state);
        return token;
    }
}

static bool is_hex(char c) {
    return c >= '0' && c <= '9' || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F';
}

// next_str returns the next string literal in the source code.
static TokenKind next_str(LexerState* state) {
    TokenKind token = TOKEN_ERROR;

    char c = get_chr(state, 0);
    if (c != '"') {
        return token;
    }

    state->current++;
    state->column++;

    // str_char* '"'
    while (true) {
        c = get_chr(state, 0);
        switch (c) {
            case '\\':
                state->current++;
                state->column++;
                c = get_chr(state, 0);
                switch (c) {
                    case 'a':
                    case 'b':
                    case 'f':
                    case 'n':
                    case 'r':
                    case 't':
                    case 'v':
                    case '\\':
                    case '\'':
                    case '"':
                        state->current++;
                        state->column++;
                        break;
                    case 'x':
                        state->current++;
                        state->column++;
                        if (!is_hex(get_chr(state, 0))) {
                            token = TOKEN_ERROR;
                            goto done;
                        }
                        state->current++;
                        state->column++;
                        if (!is_hex(get_chr(state, 0))) {
                            token = TOKEN_ERROR;
                            goto done;
                        }
                        state->current++;
                        state->column++;
                        break;
                    case 'u':
                        state->current++;
                        state->column++;
                        for (int i = 0; i < 4; i++) {
                            if (!is_hex(get_chr(state, 0))) {
                                token = TOKEN_ERROR;
                                goto done;
                            }
                            state->current++;
                            state->column++;
                        }
                        break;
                    case 'U':
                        state->current++;
                        state->column++;
                        for (int i = 0; i < 8; i++) {
                            if (!is_hex(get_chr(state, 0))) {
                                token = TOKEN_ERROR;
                                goto done;
                            }
                            state->current++;
                            state->column++;
                        }
                        break;
                    default:
                        token = TOKEN_ERROR;
                        goto done;
                }
                break;
            case '"':
                state->current++;
                state->column++;
                token = TOKEN_STR_LITERAL;
                skip(state);
                goto done;
            case '\n':
                token = TOKEN_ERROR;
                goto done;
            default:
                state->current++;
                state->column++;
                break;
        }
    }
done:
    return token;
}

static bool is_end_of_word(char c) {
    return !(
        (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        c == '_'
    );
}

// next_keyword returns the next keyword token in the given parser state.
// TODO: we can move state forward here on an error.
static TokenKind next_keyword(LexerState* state) {
#define NOT_KEYWORD return TOKEN_ERROR;

    switch (get_chr(state, 0)) {
        case 'a':
            switch (get_chr(state, 1)) {
                case 'n':
                    switch (get_chr(state, 2)) {
                        case 'd':
                            if (is_end_of_word(get_chr(state, 3))) {
                                state->current += 3;
                                state->column += 3;
                                return TOKEN_AND;
                            } else NOT_KEYWORD;
                        default: NOT_KEYWORD;
                    }
                default: NOT_KEYWORD;
            }
        case 'b':
            switch (get_chr(state, 1)) {
                case 'r':
                    switch (get_chr(state, 2)) {
                        case 'e':
                            switch (get_chr(state, 3)) {
                                case 'a':
                                    switch (get_chr(state, 4)) {
                                        case 'k':
                                            if (is_end_of_word(get_chr(state, 5))) {
                                                state->current += 5;
                                                state->column += 5;
                                                return TOKEN_BREAK;
                                            } else NOT_KEYWORD;
                                        default: NOT_KEYWORD;
                                    }
                                default: NOT_KEYWORD;
                            }
                        default: NOT_KEYWORD;
                    }
                default: NOT_KEYWORD;
            }
        case 'c':
            switch (get_chr(state, 1)) {
                case 'a':
                    switch (get_chr(state, 2)) {
                        case 's':
                            switch (get_chr(state, 3)) {
                                case 'e':
                                    if (is_end_of_word(get_chr(state, 4))) {
                                        state->current += 4;
                                        state->column += 4;
                                        return TOKEN_CASE;
                                    } else NOT_KEYWORD;
                                default: NOT_KEYWORD;
                            }
                        case 't':
                            switch (get_chr(state, 3)) {
                                case 'c':
                                    switch (get_chr(state, 4)) {
                                        case 'h':
                                            if (is_end_of_word(get_chr(state, 5))) {
                                                state->current += 5;
                                                state->column += 5;
                                                return TOKEN_CATCH;
                                            } else NOT_KEYWORD;
                                        default: NOT_KEYWORD;
                                    }
                                default: NOT_KEYWORD;
                            }
                        default: NOT_KEYWORD;
                    }
                case 'o':
                    switch (get_chr(state, 2)) {
                        case 'n':
                            switch (get_chr(state, 3)) {
                                case 's':
                                    switch (get_chr(state, 4)) {
                                        case 't':
                                            if (is_end_of_word(get_chr(state, 5))) {
                                                state->current += 5;
                                                state->column += 5;
                                                return TOKEN_CONST;
                                            } else NOT_KEYWORD;
                                        default: NOT_KEYWORD;
                                    }
                                case 't':
                                    switch (get_chr(state, 4)) {
                                        case 'i':
                                            switch (get_chr(state, 5)) {
                                                case 'n':
                                                    switch (get_chr(state, 6)) {
                                                        case 'u':
                                                            switch (get_chr(state, 7)) {
                                                                case 'e':
                                                                    if (is_end_of_word(get_chr(state, 8))) {
                                                                        state->current += 8;
                                                                        state->column += 8;
                                                                        return TOKEN_CONTINUE;
                                                                    } else NOT_KEYWORD;
                                                                default: NOT_KEYWORD;
                                                            }
                                                        default: NOT_KEYWORD;
                                                    }
                                                default: NOT_KEYWORD;
                                            }
                                        default: NOT_KEYWORD;
                                    }
                                default: NOT_KEYWORD;
                            }
                        default: NOT_KEYWORD;
                    }
                default: NOT_KEYWORD;
            }
        case 'd':
            switch (get_chr(state, 1)) {
                case 'e':
                    switch (get_chr(state, 2)) {
                        case 'f':
                            switch (get_chr(state, 3)) {
                                case 'e':
                                    switch (get_chr(state, 4)) {
                                        case 'r':
                                            if (is_end_of_word(get_chr(state, 5))) {
                                                state->current += 5;
                                                state->column += 5;
                                                return TOKEN_DEFER;
                                            } else NOT_KEYWORD;
                                        default: NOT_KEYWORD;
                                    }
                                default:
                                    if (is_end_of_word(get_chr(state, 3))) {
                                        state->current += 3;
                                        state->column += 3;
                                        return TOKEN_DEF;
                                    } else NOT_KEYWORD;
                            }
                        default: NOT_KEYWORD;
                    }
                default: NOT_KEYWORD;
            }
        case 'e':
            switch (get_chr(state, 1)) {
                case 'l':
                    switch (get_chr(state, 2)) {
                        case 's':
                            switch (get_chr(state, 3)) {
                                case 'e':
                                    if (is_end_of_word(get_chr(state, 4))) {
                                        state->current += 4;
                                        state->column += 4;
                                        return TOKEN_ELSE;
                                    } else NOT_KEYWORD;
                                default: NOT_KEYWORD;
                            }
                        case 'i':
                            switch (get_chr(state, 3)) {
                                case 'f':
                                    if (is_end_of_word(get_chr(state, 4))) {
                                        state->current += 4;
                                        state->column += 4;
                                        return TOKEN_ELIF;
                                    } else NOT_KEYWORD;
                                default: NOT_KEYWORD;
                            }
                        default: NOT_KEYWORD;
                    }
                case 'n':
                    switch (get_chr(state, 2)) {
                        case 'u':
                            switch (get_chr(state, 3)) {
                                case 'm':
                                    if (is_end_of_word(get_chr(state, 4))) {
                                        state->current += 4;
                                        state->column += 4;
                                        return TOKEN_ENUM;
                                    } else NOT_KEYWORD;
                                default: NOT_KEYWORD;
                            }
                        default: NOT_KEYWORD;
                    }
                default: NOT_KEYWORD;
            }
        case 'f':
            switch (get_chr(state, 1)) {
                case 'o':
                    switch (get_chr(state, 2)) {
                        case 'r':
                            if (is_end_of_word(get_chr(state, 3))) {
                                state->current += 3;
                                state->column += 3;
                                return TOKEN_FOR;
                            } else NOT_KEYWORD;
                        default: NOT_KEYWORD;
                    }
                default: NOT_KEYWORD;
            }
        case 'i':
            switch (get_chr(state, 1)) {
                case 'f':
                    if (is_end_of_word(get_chr(state, 2))) {
                        state->current += 2;
                        state->column += 2;
                        return TOKEN_IF;
                    } else NOT_KEYWORD;
                case 'm':
                    switch (get_chr(state, 2)) {
                        case 'p':
                            switch (get_chr(state, 3)) {
                                case 'o':
                                    switch (get_chr(state, 4)) {
                                        case 'r':
                                            switch (get_chr(state, 5)) {
                                                case 't':
                                                    if (is_end_of_word(get_chr(state, 6))) {
                                                        state->current += 6;
                                                        state->column += 6;
                                                        return TOKEN_IMPORT;
                                                    } else NOT_KEYWORD;
                                                default: NOT_KEYWORD;
                                            }
                                        default: NOT_KEYWORD;
                                    }
                                default: NOT_KEYWORD;
                            }
                        default: NOT_KEYWORD;
                    }
                default: NOT_KEYWORD;
            }
        case 'o':
            switch (get_chr(state, 1)) {
                case 'r':
                    if (is_end_of_word(get_chr(state, 2))) {
                        state->current += 2;
                        state->column += 2;
                        return TOKEN_OR;
                    } else NOT_KEYWORD;
                default: NOT_KEYWORD;
            }
        case 'r':
            switch (get_chr(state, 1)) {
                case 'e':
                    switch (get_chr(state, 2)) {
                        case 't':
                            switch (get_chr(state, 3)) {
                                case 'u':
                                    switch (get_chr(state, 4)) {
                                        case 'r':
                                            switch (get_chr(state, 5)) {
                                                case 'n':
                                                    if (is_end_of_word(get_chr(state, 6))) {
                                                        state->current += 6;
                                                        state->column += 6;
                                                        return TOKEN_RETURN;
                                                    } else NOT_KEYWORD;
                                                default: NOT_KEYWORD;
                                            }
                                        default: NOT_KEYWORD;
                                    }
                                default: NOT_KEYWORD;
                            }
                        default: NOT_KEYWORD;
                    }
                default: NOT_KEYWORD;
            }
        case 's':
            switch (get_chr(state, 1)) {
                case 't':
                    switch (get_chr(state, 2)) {
                        case 'r':
                            switch (get_chr(state, 3)) {
                                case 'u':
                                    switch (get_chr(state, 4)) {
                                        case 'c':
                                            switch (get_chr(state, 5)) {
                                                case 't':
                                                    if (is_end_of_word(get_chr(state, 6))) {
                                                        state->current += 6;
                                                        state->column += 6;
                                                        return TOKEN_STRUCT;
                                                    } else NOT_KEYWORD;
                                                default: NOT_KEYWORD;
                                            }
                                        default: NOT_KEYWORD;
                                    }
                                default: NOT_KEYWORD;
                            }
                        default: NOT_KEYWORD;
                    }
                case 'w':
                    switch (get_chr(state, 2)) {
                        case 'i':
                            switch (get_chr(state, 3)) {
                                case 't':
                                    switch (get_chr(state, 4)) {
                                        case 'c':
                                            switch (get_chr(state, 5)) {
                                                case 'h':
                                                    if (is_end_of_word(get_chr(state, 6))) {
                                                        state->current += 6;
                                                        state->column += 6;
                                                        return TOKEN_SWITCH;
                                                    } else NOT_KEYWORD;
                                                default: NOT_KEYWORD;
                                            }
                                        default: NOT_KEYWORD;
                                    }
                                default: NOT_KEYWORD;
                            }
                        default: NOT_KEYWORD;
                    }
                default: NOT_KEYWORD;
            }
        case 't':
            switch (get_chr(state, 1)) {
                case 'e':
                    switch (get_chr(state, 2)) {
                        case 's':
                            switch (get_chr(state, 3)) {
                                case 't':
                                    if (is_end_of_word(get_chr(state, 4))) {
                                        state->current += 4;
                                        state->column += 4;
                                        return TOKEN_TEST;
                                    } else NOT_KEYWORD;
                                default: NOT_KEYWORD;
                            }
                        default: NOT_KEYWORD;
                    }
                case 'r':
                    switch (get_chr(state, 2)) {
                        case 'y':
                            if (is_end_of_word(get_chr(state, 3))) {
                                state->current += 3;
                                state->column += 3;
                                return TOKEN_TRY;
                            } else NOT_KEYWORD;
                        default: NOT_KEYWORD;
                    }
                default: NOT_KEYWORD;
            }
        case 'v':
            switch (get_chr(state, 1)) {
                case 'a':
                    switch (get_chr(state, 2)) {
                        case 'r':
                            if (is_end_of_word(get_chr(state, 3))) {
                                state->current += 3;
                                state->column += 3;
                                return TOKEN_VAR;
                            } else NOT_KEYWORD;
                        default: NOT_KEYWORD;
                    }
                default: NOT_KEYWORD;
            }
        case 'w':
            switch (get_chr(state, 1)) {
                case 'h':
                    switch (get_chr(state, 2)) {
                        case 'i':
                            switch (get_chr(state, 3)) {
                                case 'l':
                                    switch (get_chr(state, 4)) {
                                        case 'e':
                                            if (is_end_of_word(get_chr(state, 5))) {
                                                state->current += 5;
                                                state->column += 5;
                                                return TOKEN_WHILE;
                                            } else NOT_KEYWORD;
                                        default: NOT_KEYWORD;
                                    }
                                default: NOT_KEYWORD;
                            }
                        default: NOT_KEYWORD;
                    }
                default: NOT_KEYWORD;
            }
        default: NOT_KEYWORD;
    }
}

// next_identifier returns the next identifier in the source code.
static TokenKind next_identifier(LexerState* state) {
    // [A-Za-z_][A-Za-z0-9_]* skip 
    char c = get_chr(state, 0);
    if (isalpha(c) || c == '_') {
        state->current++;
        state->column++;
        while (isalnum(c = get_chr(state, 0)) || c == '_') {
            state->current++;
            state->column++;
        }
        return TOKEN_IDENTIFIER;
    }
    skip(state);
    return TOKEN_ERROR;
}

// next_operator returns the next operator in the source code.
static TokenKind next_operator(LexerState* state) {
    // [!%&()*+,\-./:;<=>?[\]^{|}~] skip
    char c0 = get_chr(state, 0);
    char c1 = get_chr(state, 1);
    char c2 = get_chr(state, 2);
    TokenKind token = TOKEN_ERROR;

    switch (c0) {
        case '&':
            switch (c1) {
                case '=':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_AMPERSANDEQUAL;
                    goto done;
                default:
                    state->current++;
                    state->column++;
                    token = TOKEN_AMPERSAND;
                    goto done;
            }
        case '*':
            switch (c1) {
                case '*':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_ASTERISK2;
                    goto done;
                case '=':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_ASTERISKEQUAL;
                    goto done;
                default:
                    state->current++;
                    state->column++;
                    token = TOKEN_ASTERISK;
                    goto done;
            }
        case '^':
            switch (c1) {
                case '=':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_CARETEQUAL;
                    goto done;
                default:
                    state->current++;
                    state->column++;
                    token = TOKEN_CARET;
                    goto done;
            }
        case ':':
            state->current++;
            state->column++;
            token = TOKEN_COLON;
            goto done;
        case ',':
            state->current++;
            state->column++;
            token = TOKEN_COMMA;
            goto done;
        case '.':
            switch (c1) {
                case '.':
                    switch (c2) {
                        case '.':
                            state->current += 3;
                            state->column += 3;
                            token = TOKEN_DOT3;
                            goto done;
                        default:
                            state->current += 2;
                            state->column += 2;
                            token = TOKEN_DOT2;
                            goto done;
                    }
                case '*':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_DOTASTERISK;
                    goto done;
                case '?':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_DOTQUESTION;
                    goto done;
                default:
                    state->current++;
                    state->column++;
                    token = TOKEN_DOT;
                    goto done;
            }
        case '=':
            switch (c1) {
                case '=':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_EQUAL2;
                    goto done;
                case '>':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_EQUALRARROW;
                    goto done;
                default:
                    state->current++;
                    state->column++;
                    token = TOKEN_EQUAL;
                    goto done;
            }
        case '!':
            switch (c1) {
                case '=':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_EXCLAMEQUAL;
                    goto done;
                default:
                    state->current++;
                    state->column++;
                    token = TOKEN_EXCLAM;
                    goto done;
            }
        case '<':
            switch (c1) {
                case '<':
                    switch (c2) {
                        case '=':
                            state->current += 3;
                            state->column += 3;
                            token = TOKEN_LARROW2EQUAL;
                            goto done;
                        default:
                            state->current += 2;
                            state->column += 2;
                            token = TOKEN_LARROW2;
                            goto done;
                    }
                case '=':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_LARROWEQUAL;
                    goto done;
                default:
                    state->current++;
                    state->column++;
                    token = TOKEN_LARROW;
                    goto done;
            }
        case '{':
            state->current++;
            state->column++;
            token = TOKEN_LBRACE;
            goto done;
        case '[':
            state->current++;
            state->column++;
            token = TOKEN_LBRACKET;
            goto done;
        case '(':
            state->current++;
            state->column++;
            token = TOKEN_LPAREN;
            goto done;
        case '-':
            switch (c1) {
                case '=':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_MINUSEQUAL;
                    goto done;
                case '>':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_MINUSRARROW;
                    goto done;
                default:
                    state->current++;
                    state->column++;
                    token = TOKEN_MINUS;
                    goto done;
            }
        case '%':
            switch (c1) {
                case '=':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_PERCENTEQUAL;
                    goto done;
                default:
                    state->current++;
                    state->column++;
                    token = TOKEN_PERCENT;
                    goto done;
            }
        case '|':
            switch (c1) {
                case '=':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_PIPEEQUAL;
                    goto done;
                default:
                    state->current++;
                    state->column++;
                    token = TOKEN_PIPE;
                    goto done;
            }
        case '+':
            switch (c1) {
                case '=':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_PLUSEQUAL;
                    goto done;
                case '+':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_PLUS2;
                    goto done;
                default:
                    state->current++;
                    state->column++;
                    token = TOKEN_PLUS;
                    goto done;
            }
        case '?':
            state->current++;
            state->column++;
            token = TOKEN_QUESTION;
            goto done;
        case '>':
            switch (c1) {
                case '>':
                    switch (c2) {
                        case '=':
                            state->current += 3;
                            state->column += 3;
                            token = TOKEN_RARROW2EQUAL;
                            goto done;
                        default:
                            state->current += 2;
                            state->column += 2;
                            token = TOKEN_RARROW2;
                            goto done;
                    }
                case '=':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_RARROWEQUAL;
                    goto done;
                default:
                    state->current++;
                    state->column++;
                    token = TOKEN_RARROW;
                    goto done;
            }
        case '}':
            state->current++;
            state->column++;
            token = TOKEN_RBRACE;
            goto done;
        case ']':
            state->current++;
            state->column++;
            token = TOKEN_RBRACKET;
            goto done;
        case ')':
            state->current++;
            state->column++;
            token = TOKEN_RPAREN;
            goto done;
        case ';':
            state->current++;
            state->column++;
            token = TOKEN_SEMICOLON;
            goto done;
        case '/':
            switch (c1) {
                case '=':
                    state->current += 2;
                    state->column += 2;
                    token = TOKEN_SLASHEQUAL;
                    goto done;
                default:
                    state->current++;
                    state->column++;
                    token = TOKEN_SLASH;
                    goto done;
            }
        case '~':
            state->current++;
            state->column++;
            token = TOKEN_TILDE;
            goto done;
        default: return TOKEN_ERROR;
    }
done:
    skip(state);
    return token;
}

// next_token returns the next token in the stream.
static TokenKind next_token(LexerState* state) {
    char c = get_chr(state, 0);
    TokenKind token = TOKEN_ERROR;
    switch (c) {
        case '\0': return TOKEN_EOF;
        case ' ': case '\t': case '\r': case '\n': case '#': skip(state); return next_token(state);
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case '_':
            token = next_keyword(state);
            if (token == TOKEN_ERROR)
                token = next_identifier(state);
            return token;
        case '0': case '1': case '2': case '3': case '4': case '5': case '6':
        case '7': case '8': case '9':
            return next_num(state);
        case '&': case '*': case '^': case ':': case ',': case '.': case '=':
        case '!': case '<': case '{': case '[': case '(': case '-': case '%':
        case '|': case '+': case '?': case '>': case '}': case ']': case ')':
        case ';': case '/': case '~':
            return next_operator(state);
        case '"':
            return next_str(state);
        default:
            return TOKEN_ERROR;
    }
}


#ifdef LEXER_TEST
int main(int argc, char **argv) {
    assert(strcmp(get_tok_name(TOKEN_ERROR), "TOKEN_ERROR") == 0);

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

    LEXER_TEST_PASS("", TOKEN_EOF, 0);
    LEXER_TEST_PASS(" \t\r\n", TOKEN_EOF, 4);
    LEXER_TEST_PASS("# comment", TOKEN_EOF, 9);
    LEXER_TEST_PASS("# comment\n", TOKEN_EOF, 10);
    LEXER_TEST_PASS("42", TOKEN_INT_LITERAL, 2);
    LEXER_TEST_PASS("42   ", TOKEN_INT_LITERAL, 5);
    LEXER_TEST_PASS("0b01001", TOKEN_INT_LITERAL, 7);
    LEXER_TEST_FAILED("0b010012", LEXER_EBINCHR, 7);
    LEXER_TEST_FAILED("0b201001", LEXER_EBINCHR, 2);
    LEXER_TEST_PASS("0o777", TOKEN_INT_LITERAL, 5);
    LEXER_TEST_FAILED("0o7778", LEXER_EOCTCHR, 5);
    LEXER_TEST_PASS("0x000A", TOKEN_INT_LITERAL, 6);
    LEXER_TEST_PASS("\"你好世界\"", TOKEN_STR_LITERAL, 6);

    return 0;
}
#endif

#ifdef LEXER_EXAMPLE
int main(int argc, char **argv) {
    return 0;
}
#endif
