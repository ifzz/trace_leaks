/**
 * Copyright(c) 2017 Atanas Filipov <it.feel.filipov@gmail.com>.
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
 * DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "debugmalloc.h"
#include "debugnew.h"
#include "debuginfo.h"

#include <sys/types.h>
#include <unistd.h>

/* Prototypes for __malloc_hook, __free_hook */
#include <stdio.h>
#include <malloc.h>

extern bool m_ready;

static void* my_malloc_hook(size_t size, const void *caller);
static void* my_realloc_hook(void *ptr, size_t size, const void *caller);
static void  my_free_hook(void *ptr, const void *caller);

typedef void* (*malloc_hook_t) (size_t size, const void *caller);
typedef void  (*realloc_hook_t) (void *ptr, size_t size, const void *caller);
typedef void  (*free_hook_t) (void *ptr, const void *caller);

malloc_hook_t  org_malloc_hook;
realloc_hook_t org_realloc_hook;
free_hook_t    org_free_hook;

static void* my_malloc_hook(size_t size, const void *caller)
{
    void *result;

    /* Restore all old hooks */
    __malloc_hook = org_malloc_hook;

    /* Call recursively */
    result = malloc(size);

    /* Save underlying hooks */
    org_malloc_hook = __malloc_hook;

    if (m_ready) {
        tracer->AddEntry(result, getpid(), size, __builtin_return_address(0));
    }

    /* Restore our own hooks */
    __malloc_hook = my_malloc_hook;

    return result;
}

static void* my_realloc_hook(void *ptr, size_t size, const void *caller)
{
    void *result;

    /* Restore all old hooks */
    __realloc_hook = org_realloc_hook;

    /* Call recursively */
    result = realloc(ptr, size);

    /* Save underlying hooks */
    org_realloc_hook = __realloc_hook;

    if (m_ready) {
        tracer->AddEntry(result, getpid(), size, __builtin_return_address(0));
    }

    /* Restore our own hooks */
    __realloc_hook = my_realloc_hook;

    return result;
}

static void my_free_hook(void *ptr, const void *caller)
{
    /* Restore all old hooks */
    __free_hook = org_free_hook;

    /* Call recursively */
    free(ptr);

    /* Save underlying hooks */
    org_free_hook = __free_hook;

    if (m_ready) {
	    tracer->DelEntry(ptr);
    }

    /* Restore our own hooks */
    __free_hook = my_free_hook;
}

MallocTracer::MallocTracer()
{
    __malloc_hook  = my_malloc_hook;
    __realloc_hook = my_realloc_hook;
    __free_hook    = my_free_hook;
}

MallocTracer::~MallocTracer()
{
    __malloc_hook  = org_malloc_hook;
    __realloc_hook = org_realloc_hook;
    __free_hook    = org_free_hook;
}

