/**
 * lexer.h
 *
 * Copyright (c) 2023 Ju Lin
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of 
 * this software and associated documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the 
 * Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Lexer is a hand-written tokenizer for the So programming language.
 *
 */

#ifndef LEXER_H
#define LEXER_H

// Token is an enum of all the tokens that can be returned
// by `next_token()`.
typedef enum TokenKind {
    // This is a special one that indicates a failed tokenization.
    TOKEN_ERROR,
    // This is a special one that indicates no more characters
    // from the input. It is returned by `next_token()` when
    // it reaches the end of the file.
    TOKEN_EOF,

    // "Hello, world!"
    TOKEN_STRING_LITERAL,
    // 42, 0x2A, 0b101010, 0o42
    TOKEN_INT_LITERAL,
    // 3.14, 1.0e-10
    TOKEN_FLOAT_LITERAL,
    // foo, bar, baz
    TOKEN_IDENTIFIER,

    // &, &=
    TOKEN_AMPERSAND,
    TOKEN_AMPERSANDEQUAL,
    // *, **, *=
    TOKEN_ASTERISK,
    TOKEN_ASTERISK2,
    TOKEN_ASTERISKEQUAL,
    // ^, ^=
    TOKEN_CARET,
    TOKEN_CARETEQUAL,
    // :
    TOKEN_COLON,
    // ,
    TOKEN_COMMA,
    // .   ..   ...
    TOKEN_DOT,
    TOKEN_DOT2,
    TOKEN_DOT3,
    // .*, .?
    TOKEN_DOTASTERISK,
    TOKEN_DOTQUESTION,
    // =, ==, =>
    TOKEN_EQUAL,
    TOKEN_EQUAL2,
    TOKEN_EQUALRARROW,
    // !, !=
    TOKEN_EXCLAM,
    TOKEN_EXCLAMEQUAL,
    // >, >=, >>, >>=
    TOKEN_LARROW,
    TOKEN_LARROW2,
    TOKEN_LARROW2EQUAL,
    TOKEN_LARROWEQUAL,
    // [, {, (
    TOKEN_LBRACE,
    TOKEN_LBRACKET,
    TOKEN_LPAREN,
    // -, -=, ->
    TOKEN_MINUS,
    TOKEN_MINUSEQUAL,
    TOKEN_MINUSRARROW,
    // %, %=
    TOKEN_PERCENT,
    TOKEN_PERCENTEQUAL,
    // |, |=
    TOKEN_PIPE,
    TOKEN_PIPEEQUAL,
    // +, ++, +=
    TOKEN_PLUS,
    TOKEN_PLUS2,
    TOKEN_PLUSEQUAL,
    // ?
    TOKEN_QUESTION,
    // >, >=, >>, >>=
    TOKEN_RARROW,
    TOKEN_RARROW2,
    TOKEN_RARROW2EQUAL,
    TOKEN_RARROWEQUAL,
    // ], }, )
    TOKEN_RBRACE,
    TOKEN_RBRACKET,
    TOKEN_RPAREN,
    // ;
    TOKEN_SEMICOLON,
    // /, /=
    TOKEN_SLASH,
    TOKEN_SLASHEQUAL,
    // ~
    TOKEN_TILDE,

    // Reserved keywords
    TOKEN_AND,
    TOKEN_BREAK,
    TOKEN_CASE,
    TOKEN_CATCH,
    TOKEN_CONST,
    TOKEN_CONTINUE,
    TOKEN_DEF,
    TOKEN_DEFER,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_ENUM,
    TOKEN_FOR,
    TOKEN_IF,
    TOKEN_IMPORT,
    TOKEN_OR,
    TOKEN_RETURN,
    TOKEN_STRUCT,
    TOKEN_SWITCH,
    TOKEN_TEST,
    TOKEN_TRY,
    TOKEN_VAR,
    TOKEN_WHILE,
} TokenKind;

// TokenName is a string representation of each token
// kind. It is used for debugging.
char* TokenName[] = {
    "TOKEN_ERROR",
    "TOKEN_EOF",

    "TOKEN_STRING_LITERAL",
    "TOKEN_INT_LITERAL",
    "TOKEN_FLOAT_LITERAL",
    "TOKEN_IDENTIFIER",

    "TOKEN_AMPERSAND",
    "TOKEN_AMPERSANDEQUAL",
    "TOKEN_ASTERISK",
    "TOKEN_ASTERISK2",
    "TOKEN_ASTERISKEQUAL",
    "TOKEN_CARET",
    "TOKEN_CARETEQUAL",
    "TOKEN_COLON",
    "TOKEN_COMMA",
    "TOKEN_DOT",
    "TOKEN_DOT2",
    "TOKEN_DOT3",
    "TOKEN_DOTASTERISK",
    "TOKEN_DOTQUESTION",
    "TOKEN_EQUAL",
    "TOKEN_EQUAL2",
    "TOKEN_EQUALRARROW",
    "TOKEN_EXCLAM",
    "TOKEN_EXCLAMEQUAL",
    "TOKEN_LARROW",
    "TOKEN_LARROW2",
    "TOKEN_LARROW2EQUAL",
    "TOKEN_LARROWEQUAL",
    "TOKEN_LBRACE",
    "TOKEN_LBRACKET",
    "TOKEN_LPAREN",
    "TOKEN_MINUS",
    "TOKEN_MINUSEQUAL",
    "TOKEN_MINUSRARROW",
    "TOKEN_PERCENT",
    "TOKEN_PERCENTEQUAL",
    "TOKEN_PIPE",
    "TOKEN_PIPEEQUAL",
    "TOKEN_PLUS",
    "TOKEN_PLUS2",
    "TOKEN_PLUSEQUAL",
    "TOKEN_QUESTIONMARK",
    "TOKEN_RARROW",
    "TOKEN_RARROW2",
    "TOKEN_RARROW2EQUAL",
    "TOKEN_RARROWEQUAL",
    "TOKEN_RBRACE",
    "TOKEN_RBRACKET",
    "TOKEN_RPAREN",
    "TOKEN_SEMICOLON",
    "TOKEN_SLASH",
    "TOKEN_SLASHEQUAL",
    "TOKEN_TILDE",

    "TOKEN_AND",
    "TOKEN_BREAK",
    "TOKEN_CASE",
    "TOKEN_CATCH",
    "TOKEN_CONST",
    "TOKEN_CONTINUE",
    "TOKEN_DEF",
    "TOKEN_DEFER",
    "TOKEN_ELIF",
    "TOKEN_ELSE",
    "TOKEN_ENUM",
    "TOKEN_FOR",
    "TOKEN_IF",
    "TOKEN_IMPORT",
    "TOKEN_OR",
    "TOKEN_RETURN",
    "TOKEN_STRUCT",
    "TOKEN_SWITCH",
    "TOKEN_TEST",
    "TOKEN_TRY",
    "TOKEN_VAR",
    "TOKEN_WHILE",
};

typedef enum LexerError {
    LEXER_EOK,
    LEXER_EBINCHR,
    LEXER_EOCTCHR,
    LEXER_EHEXCHR,
} LexerError;

// LexerState is the state of the tokenizer.
// It contains the state of the source code and
// the current position in the source code.
// It can also be used to keep track of the current line
// and column.
// The tokenizer state avoids having global variables.
typedef struct LexerState {
    // The source code.
    char*  source;
    // The current position in the source code.
    char*  current;
    // The current line (1-based).
    int    line;
    // The current column (1-based).
    int    column;
    // The error.
    LexerError error;
} LexerState;


#endif
