/* SDSLib, A C dynamic strings library
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "sds.h"

#include <string.h>

#include "zmalloc.h"

sds sdsnewlen(const void *init, size_t initlen) {
    struct sdshdr *sh;

    if (init) {
        sh = zmalloc(sizeof(struct sdshdr) + initlen + 1);
    } else {
        sh = zcalloc(sizeof(struct sdshdr) + initlen + 1);
    }
    if (sh == NULL) {
        return NULL;
    }
    // if init=null, sds initialize with zero, so free=0.
    sh->len = initlen;
    sh->free = 0;
    if (init && initlen) {
        memcpy(sh->buf, init, initlen);
    }
    sh->buf[initlen] = '\0';
    return sh->buf;
}

sds sdsnew(const char *init) {
    size_t len = (init == NULL) ? 0 : strlen(init);
    return sdsnewlen(init, len);
}

sds sdsempty() {
    return sdsnewlen("", 0);
}

sds sdsdup(const sds s) {
    return sdsnewlen(s, sdslen(s));
}

void sdsfree(sds s) {
    if (s == NULL) return;
    zfree(s - sizeof(struct sdshdr));
}

sds sdsgrowzero(sds s, size_t len) {
    struct sdshdr *sh = (void *)(s - sizeof(struct sdshdr));
    size_t oldlen = sh->len;
    if (len <= oldlen) {
        return s;
    }
    if (len <= oldlen + sh->free) {
        memset(sh->buf + oldlen, 0, (len - oldlen));
        sh->len = len;
        sh->free = sh->free - len + oldlen;
        sh->buf[len] = '\0';
    } else {
        sh = zrealloc(sh, len);
        if (sh == NULL) {
            return NULL;
        }
        sh->len = len;
        sh->free = 0;
        sh->buf[len] = '\0';
    }
    return sh->buf;
}

sds sdscatlen(sds s, const void *t, size_t len) {
    struct sdshdr *sh = (void *)(s - sizeof(struct sdshdr));
    if (len <= sh->free) {
        memcpy(sh->buf, t, len);
        sh->len += len;
        sh->free -= len;
    } else {
        size_t oldlen = sh->len;
        sh = zrealloc(sh, oldlen + len);
        if (sh == NULL) {
            return NULL;
        }
        memcpy(sh->buf + oldlen, t, len);
        sh->len = oldlen + len;
        sh->free = 0;
    }
    sh->buf[sh->len] = '\0';
    return sh->buf;
}

sds sdscat(sds s, const char *t) {
    return sdscatlen(s, t, strlen(t));
}

sds sdscatsds(sds s, const sds t) {
    return sdscatlen(s, t, sdslen(t));
}

sds sdscpylen(sds s, const void *t, size_t len) {
    if (s == NULL) {
        return sdsnewlen(t, len);
    } else {
        struct sdshdr *sh = (void *)(s - sizeof(struct sdshdr));
        if (len <= sh->len + sh->free) {
            memcmp(sh->buf, t, len);
            sh->free = sh->free + sh->len - len;
            sh->len = len;
        } else {
            sh = zrealloc(sh, len);
            if (sh == NULL) {
                return NULL;
            }
            memcpy(sh, t, len);
            sh->len = len;
            sh->free = 0;
        }
        sh->buf[sh->len] = '\0';
        return sh->buf;
    }
}

sds sdscpy(sds s, const char *t) {
    return sdscpylen(s, t, strlen(t));
}

int main(void) {
    struct sdshdr *sh;
    sds x = sdsnew("foo"), y;

    memcmp(x, "foo\0", 4);

    sdsfree(x);
    x = sdsnewlen("foo", 2);
    x = sdscat(x, "bar");
    size_t len = sdslen(x);
    x = sdscpy(x, "a");
    len = sdslen(x);  //1
    x = sdscpy(x, "xyzxxxxxxxxxxyyyyyyyyyykkkkkkkkkk");
    len = sdslen(x);  // 33
    sdsfree(x);
}