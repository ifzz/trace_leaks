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

#include "debugnew.h"
#include "debuginfo.h"

#include <sstream>
#include <new>

#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <cassert>

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

bool m_ready;

void* operator new(std::size_t count)
{
    void* p = malloc(count);
    if (m_ready) {
        tracer->AddEntry(p, getpid(), count, __builtin_return_address(0));
    }
    return p;
}

void* operator new[](std::size_t count)
{
    void* p = malloc(count);
    if (m_ready) {
        tracer->AddEntry(p, getpid(), count, __builtin_return_address(0));
    }
    return p;
}

void operator delete(void* ptr) noexcept(true)
{
	if (m_ready) {
	    tracer->DelEntry(ptr);
    }
	free(ptr);
}

void operator delete[](void* ptr) noexcept(true)
{
	if (m_ready) {
	    tracer->DelEntry(ptr);
    }
	free(ptr);
}

Tracer::Entry::Entry()
 : m_pid(0), m_size(0), m_parent(NULL)
{}

Tracer::Entry::Entry(int Pid, int Size, void* Parent)
 : m_pid(Pid), m_size(Size), m_parent(Parent)
{assert(m_pid > 0 && m_size > 0 && m_parent);
}

int Tracer::Entry::Pid() const
{
    return m_pid;
}

int Tracer::Entry::Size() const
{
    return m_size;
}

void* Tracer::Entry::Addr() const
{
    return m_parent;
}

Tracer::Lock::Lock(Tracer& tracer)
 : m_tracer(tracer)
{
	m_tracer.lock();
}

Tracer::Lock::~Lock()
{
	m_tracer.unlock();
}

Tracer::Tracer(const char* exec_name)
 :m_lockCount(0), m_exec_name(exec_name)
{assert(exec_name);

    fprintf(stderr, "Memory Tracer is active!!!\n");
    m_malloctracer = new MallocTracer;
	m_ready = true;
}

Tracer::~Tracer()
{
	m_ready = false;

	ShowLeaks();

    delete m_malloctracer;
}

void Tracer::AddEntry(void* p, int Pid, int Size, void* Parent)
{assert(p && Pid > 0 && Size > 0 && Parent);

	if (m_lockCount > 0) {
		return;
    }

	Tracer::Lock lock(*this);

	m_map[p] = Entry(Pid, Size, Parent);
}

void Tracer::DelEntry(void* p)
{assert(p);

	if (m_lockCount > 0) {
		return;
    }

	Tracer::Lock lock(*this);

	std::map<void*, Entry>::iterator it = m_map.find(p);
	if (it != m_map.end()) {
		m_map.erase(it);
	}
}

static const char* logfile = "tracer.log";

void Tracer::ShowLeaks()
{
    FILE* fh = fopen(logfile, "a+");
    if (fh) {
        fprintf(fh, "\n======== List of memory leak(s):\n");
        if (m_map.size() != 0) {
            for (std::map<void*, Entry>::iterator it = m_map.begin ();
                   it != m_map.end (); ++it)
            {
                fprintf(fh, "Leak: Pid:%6d, Chunk:%p, Size:%10d, Call:%p\n",
                        it->second.Pid(), it->first, it->second.Size(),
                        it->second.Addr());
                // Try to extract source code and line number for address
                unsigned long int addr = (unsigned long int)(it->second.Addr());
                char *sline = AddrToLine(addr, m_exec_name);
                if (strstr(sline, "??") != NULL) {
                    Dl_info info;
                    int ret = dladdr(it->second.Addr(), &info);
                    if (ret) {
                        fprintf(fh, "Shared obj:%s Base:%p Name:%s Addr:%p\n\n",
                             info.dli_fname, info.dli_fbase, info.dli_sname,
                             info.dli_saddr);
                    } else {
                        fprintf(fh, "Can't find the shared address %lx, %s\n",
                                addr, dlerror());
                    }
                } else {
                    fprintf(fh, "Source file:%s\n", sline);
                }
            }
        }
        if (fh) {
          fclose(fh);
        }
    } else {
        printf("Can't create the log:%s [%s]\n", logfile, strerror(errno));
    }
}

/* Obtain a backtrace and print it to stdout. */
void Tracer::ShowBacktrace()
{
    void *array[32];

    size_t size = backtrace(array, 32);
    char **strings = backtrace_symbols(array, size);

    printf("Obtained %zd stack frames.\n", size);

    for (size_t i = 0; i < size; i++) {
        printf ("%s\n", strings[i]);
    }

    free (strings);
}

void Tracer::lock()
{
    m_lockCount++;
}

void Tracer::unlock()
{
    m_lockCount--;
}

