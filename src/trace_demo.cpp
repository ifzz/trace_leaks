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
#include "debugmalloc.h"

#include <string.h>
#include <dlfcn.h>

#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;
static const char* LIB_NAME = "/usr/lib/libcdaudio.so.1.0.0";

/* The location of executable to be traced */
static const char* EXEC_NAME = "./bin/trace_demo";

Tracer* tracer;

class Foo
{
public:
    Foo () : _x( 1000 ) {}
    void func ()
    {
        [this] () { cout << _x << endl; } ();
    }

private:
        int _x;
};

struct MyLambda {
    void operator() (int y) { cout << y << endl; }
    void operator= (int y) {
        cout << "===" << endl;
        cout << y << endl;
        cout << "===" << endl;
    }
}MyLambda_operatorobject;

Foo f1;

int main(int argc, char** argv)
{

    tracer = new Tracer(EXEC_NAME);

    auto sum = [argc](int x, int y) { return x + y + argc; };
    cout << sum( 5, 2) << endl;
    cout << sum(10, 5) << endl;

    f1.func();

    Foo t; t.func();

    /**
     * Fake leak detection!!!
     * Source file:allocate at /usr/include/c++/4.8/ext/new_allocator.h:105
     */
    vector<int> numbers {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    for_each(numbers.begin(), numbers.end(), MyLambda_operatorobject);

    MyLambda_operatorobject(23);
    MyLambda_operatorobject = 16;

    // Create memory leaks!
    char* pc = new char;
    memset(pc, 0, sizeof(*pc));
    char* ts = new char[16];
    memset(ts, 0, sizeof(*ts));

    void* unused = malloc(13);
    memset(unused, 0, sizeof(13));

    char* temp = strdup("koko");
    if (realloc(temp, 64) != NULL) {
        printf("Temp:%s\n", temp);
    }

    void* tmp_dl = dlopen(LIB_NAME, RTLD_NOW);
    if (tmp_dl) {
      dlclose(tmp_dl);
    }

    delete(tracer);

	return 0;
}
