/*
Copyright (c) 2018, David Anderson
All rights reserved.

Redistribution and use in source and binary forms, with
or without modification, are permitted provided that the
following conditions are met:

    Redistributions of source code must retain the above
    copyright notice, this list of conditions and the following
    disclaimer.

    Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials
    provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include <stdio.h>
#include <string.h> /* memcpy */
#include <sys/types.h> /* lseek and read */
#ifdef HAVE_UNISTD_H
#include <unistd.h> /* lseek read close */
#endif /* HAVE_UNISTD_H */
#include "dwarf_types.h"
#include "dwarf_reading.h" /* for error codes */
#include "dwarf_object_read_common.h"

/*  Neither off_t nor ssize_t is in C90.
    However, both are in Posix:
    IEEE Std 1003.1-1990, aka
    ISO/IEC 9954-1:1990. */
int
dwarf_object_read_random(int fd,char *buf,off_t loc,
    size_t size,off_t filesize,int *errc)
{
    off_t scode = 0;
    ssize_t rcode = 0;
    off_t endpoint = 0;
#if 0
printf("dadebug RRMOA at location 0x%lx size 0x%lx filesz 0x%lx\n",
(unsigned long)loc,(unsigned long)size,
(unsigned long)filesize);
#endif
    if (loc >= filesize) {
        /*  Seek can seek off the end. Lets not allow that.
            The object is corrupt. */
        P("ERROR Attempted seek to %lu"
            " past file size %lu RO_SEEK_OFF_END\n",
            (unsigned long)loc,
            (unsigned long)filesize);
        *errc = RO_SEEK_OFF_END;
        return DW_DLV_ERROR;
    }
    endpoint = loc+size;
    if (endpoint > filesize) {
        /*  Let us -not- try to read past end of object.
            The object is corrupt. */
        P("ERROR Attempted READ of %lu bytes at %lu (sum: %lu)"
            " exceeds file size of %lu RO_READ_OFF_END\n",
            (unsigned long)size,(unsigned long)loc,
            (unsigned long)endpoint,
            (unsigned long)filesize);
        *errc = RO_READ_OFF_END;
        return DW_DLV_ERROR;
    }
    scode = lseek(fd,loc,SEEK_SET);
    if (scode == (off_t)-1) {
        P("ERROR Attempted SEEK to %lu "
            " failed. RO_ERR_SEEK \n",
            (unsigned long)loc);
        *errc = RO_ERR_SEEK;
        return DW_DLV_ERROR;
    }
    rcode = read(fd,buf,size);
    if (rcode == -1 ||
        (size_t)rcode != size) {
        P("ERROR Attempted READ at  %lu of %lu bytes "
            " failed. RO_ERR_READ \n",
            (unsigned long)loc,(unsigned long)size);
        *errc = RO_ERR_READ;
        return DW_DLV_ERROR;
    }
    return DW_DLV_OK;
}

/*
  A byte-swapping version of memcpy
  for cross-endian use.
  Only 2,4,8 should be lengths passed in.
*/
void *
dwarf_ro_memcpy_swap_bytes(void *s1, const void *s2, size_t len)
{
    void *orig_s1 = s1;
    unsigned char *targ = (unsigned char *) s1;
    const unsigned char *src = (const unsigned char *) s2;

    if (len == 4) {
        targ[3] = src[0];
        targ[2] = src[1];
        targ[1] = src[2];
        targ[0] = src[3];
    } else if (len == 8) {
        targ[7] = src[0];
        targ[6] = src[1];
        targ[5] = src[2];
        targ[4] = src[3];
        targ[3] = src[4];
        targ[2] = src[5];
        targ[1] = src[6];
        targ[0] = src[7];
    } else if (len == 2) {
        targ[1] = src[0];
        targ[0] = src[1];
    }
/* should NOT get below here: is not the intended use */
    else if (len == 1) {
        targ[0] = src[0];
    } else {
        memcpy(s1, s2, len);
    }
    return orig_s1;
}

void
dwarf_safe_strcpy(char *out, long outlen, const char *in, long inlen)
{
    if (inlen >= (outlen - 1)) {
        strncpy(out, in, outlen - 1);
        out[outlen - 1] = 0;
    } else {
        strcpy(out, in);
    }
}
