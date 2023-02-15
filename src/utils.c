#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

void CharBuf_Init(CharBuf* cbuf) {
    cbuf->cap = 0;
    cbuf->len = 0;
    cbuf->buf = NULL;
}

void CharBuf_Finalize(CharBuf* cbuf) {
    free(cbuf->buf);
}

bool CharBuf_Resize(CharBuf* cbuf, size_t size) {
    if (cbuf->cap < size) {
        size_t cap = cbuf->cap;
        if (cap == 0) {
            cap = 256;
        }
        while (cap < size && cap != 0) {
            cap <<= 1;
        }
        if (cap == 0) { /* overflow */
            cap = size;
        }
        char* buf = (char*)realloc(cbuf->buf, cap);
        if (buf == NULL) {
            return false;
        }
        cbuf->buf = buf;
        cbuf->cap = cap;
    }
    cbuf->len = size;
    return true;
}

void CharBuf_Append(CharBuf* cbuf, char c) {
    const size_t len = cbuf->len;
    if (!CharBuf_Resize(cbuf, len + 1)) {
        fprintf(stderr, "FATAL: out of memory\n");
        exit(1);
    }
    cbuf->buf[len] = c;
}
