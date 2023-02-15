#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Range {
    size_t start;
    size_t end;
} Range;

inline static Range Range_New(size_t start, size_t end) {
    const Range range = {start, end};
    return range;
}

typedef struct CharBuf {
    size_t cap;
    size_t len;
    char*  buf;
} CharBuf;

void CharBuf_Init(CharBuf* cbuf);
void CharBuf_Finalize(CharBuf* cbuf);
bool CharBuf_Resize(CharBuf* cbuf, size_t size);
void CharBuf_Append(CharBuf* cbuf, char c);

#ifdef __cplusplus
}
#endif

#endif
