# trace_leaks
Tracing Memory Leaks

The idea is to store information about each allocation in a global data
structure and dump its contents at the end of the program. Overloaded operator
delete would remove entries from this data structure.
Since operator delete has only access to a pointer to previously allocated
memory, we have to be able to reasonably quickly find the entry based
on this pointer.

This project based on article created by "Reliable Software"
C++ In Action: Techniques. Section "Tracing Memory Leaks".
https://www.relisoft.com/book/tech/9new.html

The authors of article are:
Bartosz Milewski, Debbie Ehrlich, Wieslaw Kalkus, Piotr Trojanowski.
