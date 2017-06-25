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

#ifndef __DEBUGNEW_H__
#define __DEBUGNEW_H__

#include "debugmalloc.h"

#include <map>
#include <string>

#include <stdio.h>
#include <stdlib.h>

// Operator overloading
void* operator new(std::size_t count);
void* operator new[](std::size_t count);
void  operator delete(void* ptr) noexcept(true);
void  operator delete[](void* ptr) noexcept(true);

class Tracer
{
private:

	class Entry
	{
	public:

		Entry();

		Entry(int Pid, int Size, void* Parent);

		int Pid() const;

		int Size() const;

        void* Addr() const;

    private:
		int   m_pid;
		int   m_size;
        void* m_parent;
    };

	class Lock
	{
	public:

		Lock(Tracer& tracer);

		~Lock();

	private:
		Tracer& m_tracer;
	};

	friend class Lock;

public:

	Tracer(const char* exec_name);

	~Tracer();

	void AddEntry(void* p, int Pid, int Size, void* Parent);

	void DelEntry(void* p);

	void ShowLeaks();

    void ShowBacktrace();

private:

	void lock();

	void unlock();

private:
	std::map<void*, Entry> m_map;
	int m_lockCount;
    const char* m_exec_name;
    MallocTracer* m_malloctracer;
};

extern Tracer* tracer;

#endif
