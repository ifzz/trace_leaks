/**
 * Copyright (c) 2017 Atanas Filipov <it.feel.filipov@gmail.com>.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "debuginfo.h"

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

/* The location of add2line utility */
static const char* ADDR_2_LINE = "/usr/bin/addr2line";

char* AddrToLine(unsigned long int addr, const char *file)
{assert(addr && file);

    static char buf[256];

    char *sline = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    // preparing defined "ADDR_2_LINE" command to be executed
    sprintf(buf, "%s -f -i -p -C -e %s -f -i %lx", ADDR_2_LINE, file, addr);

    /**
     * The popen() function opens a process by creating a bidirectional
     * pipe forking, and invoking the shell.
     */
    FILE* f = popen(buf, "r");
    if (f == NULL) {
        perror(buf);
        return NULL;
    }

    /**
     * If linep points to a NULL pointer, a new buffer will be allocated.
     * In either case, *linep and *linecapp will be updated accordingly.
     */
    while((linelen = getline(&sline, &linecap, f)) > 0) {
#ifndef NDEBUG
        fprintf(stderr, "%s The full path line length is: %zd\n", sline, linelen);
#endif
    }

    pclose(f);

    return sline;
}

